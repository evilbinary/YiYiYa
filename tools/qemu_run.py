#!/usr/bin/env python3
# coding:utf-8
"""在 qemu 里依次执行多条命令（我们的 shell 不支持 `;`，只能一条条发）。

用法：
    PLAT=raspi3 CMD="test-rand|ps" WAIT=110 python3 tools/qemu_run.py

与 qemu_ps_test.py 的区别：后者只发一条命令；这里按 `|` 拆分，每条之间
等 5 秒让上一条的输出刷出来，全部发完后多等 8 秒再退出。
"""
import os
import subprocess
import sys
import time

PLAT = os.environ.get("PLAT", "raspi3")
CMD = os.environ.get("CMD", "ps")
WAIT = int(os.environ.get("WAIT", "110"))
DISK = os.environ.get("DISK", "image/disk.img")

if PLAT == "raspi3":
    cmd = [
        "qemu-system-aarch64", "-M", "raspi3b",
        "-kernel", "build/raspi3/armv8-a/debug/kernel.elf",
        "-serial", "stdio",
        "-drive", "if=sd,id=sd0,format=raw,file=" + DISK,
        "-display", "none",
    ]
elif PLAT == "raspi5":
    cmd = [
        "qemu-system-aarch64", "-M", "raspi5b",
        "-kernel", "build/raspi5/armv8-a/debug/kernel.elf",
        "-serial", "stdio",
        "-drive", "if=sd,id=sd0,format=raw,file=" + DISK,
        "-display", "none",
    ]
else:
    print("unknown PLAT: %s" % PLAT)
    sys.exit(2)

p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                     stderr=subprocess.STDOUT, bufsize=0)
out = b""
cmds = CMD.split("|")
started = False
idx = 0
last = 0.0
start = time.time()
deadline = start + WAIT

while time.time() < deadline:
    chunk = p.stdout.read(1)
    if not chunk:
        if p.poll() is not None:
            break
        continue
    out += chunk
    text = out.decode("utf-8", errors="ignore")
    now = time.time()
    if not started and "yiyiya$" in text:
        time.sleep(2)
        started = True
        last = 0.0
    if started and idx < len(cmds) and (now - last) > 5:
        p.stdin.write((cmds[idx] + "\n").encode())
        p.stdin.flush()
        idx += 1
        last = now
    if idx >= len(cmds) and (now - last) > 8:
        break

p.kill()
sys.stdout.write(out.decode("utf-8", errors="ignore"))
