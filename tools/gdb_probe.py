#!/usr/bin/env python3
# coding:utf-8
"""用 qemu 的 gdb stub 断在指定地址，抓现场。

用法：
    python3 tools/gdb_probe.py 0x71000d44 test-rand

流程：qemu(-s + tcp serial) -> 等 shell 提示符 -> gdb 挂上去下断点
      -> 通过 serial 发命令 -> 断点命中后打印 bt/寄存器
"""
import os
import socket
import subprocess
import sys
import time

ADDR = sys.argv[1] if len(sys.argv) > 1 else "0x71000d44"
CMD = sys.argv[2] if len(sys.argv) > 2 else "test-rand"
KERNEL = "build/raspi3/armv8-a/debug/kernel.elf"
DISK = os.environ.get("DISK", "image/disk.img")
SERIAL_PORT = 5555
GDB_PORT = 1234

qemu = subprocess.Popen([
    "qemu-system-aarch64", "-M", "raspi3b",
    "-kernel", KERNEL,
    "-serial", "tcp:127.0.0.1:%d,server,nowait" % SERIAL_PORT,
    "-drive", "if=sd,id=sd0,format=raw,file=" + DISK,
    "-display", "none",
    "-gdb", "tcp::%d" % GDB_PORT,
], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
print("qemu pid", qemu.pid)

time.sleep(3)
try:
    s = socket.create_connection(("127.0.0.1", SERIAL_PORT), timeout=30)
except Exception as e:
    print("serial connect failed:", e)
    qemu.kill()
    sys.exit(1)
s.settimeout(30)

buf = b""
t0 = time.time()
while b"yiyiya$" not in buf and time.time() - t0 < 90:
    try:
        d = s.recv(8192)
        if not d:
            break
        buf += d
    except socket.timeout:
        break
print("shell ready:", b"yiyiya$" in buf, "bytes", len(buf))
if b"yiyiya$" not in buf:
    qemu.kill()
    sys.exit(1)

STEPS = int(os.environ.get("STEPS", "0"))

_steps = ""
if STEPS > 0:
    # 逐条单步并打印 PC，用来看「命中之后到底走到哪」。
    _steps = "".join(
        'stepi\nprintf "S%d pc=%%p\\n", $pc\n' % i for i in range(STEPS))

gdb_cmds = """set pagination off
set confirm off
target remote :%d
hbreak *%s
continue
printf "HIT pc=%%p lr=%%p sp=%%p\\n", $pc, $lr, $sp
%s
info registers pc lr sp x29
x/6i $pc
detach
quit
""" % (GDB_PORT, ADDR, _steps)
open("/tmp/gdb_cmds.txt", "w").write(gdb_cmds)

g = subprocess.Popen(["aarch64-none-elf-gdb", "-batch", "-x", "/tmp/gdb_cmds.txt",
                      KERNEL],
                     stdout=open("/tmp/gdb.out", "w"), stderr=subprocess.STDOUT)
time.sleep(4)
print("gdb attached")

s.sendall((CMD + "\n").encode())
print("sent:", CMD)

t0 = time.time()
while time.time() - t0 < 35:
    try:
        txt = open("/tmp/gdb.out").read()
    except Exception:
        txt = ""
    if "HIT pc" in txt or "Program received" in txt or "Cannot access" in txt:
        break
    time.sleep(1)

try:
    g.wait(timeout=15)
except Exception:
    g.kill()

print("=== gdb output ===")
print(open("/tmp/gdb.out").read()[-4000:])
qemu.kill()
