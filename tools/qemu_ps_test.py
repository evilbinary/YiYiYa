import os
import subprocess
import sys
import time

# 平台冒烟脚本：启动 QEMU -> 等 shell 提示符 -> 发一条命令 -> 收集输出 -> 退出。
#   PLAT=raspi2（默认）/ raspi3 / raspi5  选择机器与内核
#   CMD="ps"                             要执行的命令（默认 ps）
#   WAIT=45                              总超时秒数
PLAT = os.environ.get("PLAT", "raspi2")
CMD = os.environ.get("CMD", "ps")
WAIT = int(os.environ.get("WAIT", "45"))
DISK = os.environ.get("DISK", "image/disk.img")

if PLAT == "raspi2":
    cmd = [
        "qemu-system-arm",
        "-M", "raspi2b",
        "-kernel", "build/raspi2/armv7-a/debug/kernel.elf",
        "-serial", "stdio",
        "-drive", "if=sd,id=sd0,format=raw,file=" + DISK,
        "-smp", "4",
        "-nographic",
    ]
elif PLAT == "raspi3":
    cmd = [
        "qemu-system-aarch64",
        "-M", "raspi3b",
        "-kernel", "build/raspi3/armv8-a/debug/kernel.elf",
        "-serial", "stdio",
        "-drive", "if=sd,id=sd0,format=raw,file=" + DISK,
        "-device", "usb-mouse",
        "-display", "none",
        "-d", "int", "-D", "/tmp/qemu_exc.log",
    ]
elif PLAT == "raspi5":
    cmd = [
        "qemu-system-aarch64",
        "-M", "raspi5b",
        "-kernel", "build/raspi5/armv8-a/debug/kernel.elf",
        "-serial", "stdio",
        "-drive", "if=sd,id=sd0,format=raw,file=" + DISK,
        "-display", "none",
        "-d", "int", "-D", "/tmp/qemu_exc.log",
    ]
else:
    print("unknown PLAT: %s" % PLAT)
    sys.exit(2)

p = subprocess.Popen(
    cmd,
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
    bufsize=0,
)
out = b""
sent = False
sent_mark = 0
start = time.time()
deadline = start + WAIT
text = ""
while time.time() < deadline:
    chunk = p.stdout.read(1)
    if not chunk:
        if p.poll() is not None:
            break
        continue
    out += chunk
    sys.stdout.write(chunk.decode("utf-8", errors="ignore"))
    sys.stdout.flush()
    text = out.decode("utf-8", errors="ignore")
    if (not sent) and ("yiyiya$" in text or "Welcome" in text):
        time.sleep(2)
        sent_mark = len(text)
        p.stdin.write((CMD + "\n").encode())
        p.stdin.flush()
        sent = True
    if sent and (time.time() - start > WAIT - 8):
        break

p.kill()
print("\n--- captured command section (cmd=%r) ---" % CMD)
for line in text[sent_mark:].splitlines():
    if line.strip():
        print(line)
