import socket, subprocess, sys, time
HOST, PORT = "127.0.0.1", 4460
KERNEL = "/Users/evil/dev/c/YiYiYa/build/raspi3/armv8-a/debug/kernel.elf"
DISK = "/Users/evil/dev/c/YiYiYa/image/disk.img"
LOG = "/tmp/xorg_qemu.log"
QEMU_CMD = ["qemu-system-aarch64", "-name", "YiYiYa-xorg", "-M", "raspi3b", "-rtc", "base=localtime", "-kernel", KERNEL, "-serial", "tcp:127.0.0.1:%d,server,nowait" % PORT, "-drive", "if=sd,id=sd0,format=raw,file=" + DISK, "-display", "none"]
import os
if os.environ.get("GDB"):
    QEMU_CMD.insert(1, "-s")  # gdbstub on :1234


def start_qemu():
    subprocess.run(["pkill", "-9", "-f", "YiYiYa-xorg"], capture_output=True)
    time.sleep(1)
    lf = open(LOG, "wb")
    p = subprocess.Popen(QEMU_CMD, stdout=lf, stderr=lf, start_new_session=True)
    print("qemu pid", p.pid)


def connect(retries=30):
    for _ in range(retries):
        try:
            s = socket.create_connection((HOST, PORT), timeout=5)
            s.settimeout(0.5)
            return s
        except OSError:
            time.sleep(1)
    raise SystemExit("connect fail")


def boot_wait(s, timeout=90):
    buf = bytearray()
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            d = s.recv(8192)
            if d:
                buf.extend(d)
                if b"yiyiya$" in bytes(buf)[-64:]:
                    return bytes(buf)
        except socket.timeout:
            pass
        s.sendall(b"\n")
        time.sleep(1.0)
    return bytes(buf)


def handshake(s, timeout=60):
    """等启动脚本彻底跑完：反复 echo 标记直到自己收到，避免命令被 boot 脚本吃掉"""
    deadline = time.time() + timeout
    while time.time() < deadline:
        tag = int(time.time())
        s.sendall(b"echo READY_%d\n" % tag)
        mark = b"READY_%d" % tag
        buf = bytearray()
        t = time.time() + 3
        while time.time() < t:
            try:
                d = s.recv(8192)
                if d:
                    buf.extend(d)
            except socket.timeout:
                pass
        if mark in bytes(buf):
            return True
        time.sleep(1.5)
    return False


def run_cmd(s, cmd, wait=60):
    s.sendall(cmd.encode() + b"\n")
    buf = bytearray()
    deadline = time.time() + wait
    while time.time() < deadline:
        try:
            d = s.recv(8192)
            if d:
                buf.extend(d)
                if bytes(buf).rstrip().endswith(b"yiyiya$"):
                    break
        except socket.timeout:
            pass
    return bytes(buf)


def drain(s, secs=30):
    buf = bytearray()
    deadline = time.time() + secs
    while time.time() < deadline:
        try:
            d = s.recv(8192)
            if d:
                buf.extend(d)
                deadline = time.time() + 5
        except socket.timeout:
            pass
    return bytes(buf)


if __name__ == "__main__":
    start_qemu()
    s = connect()
    boot = boot_wait(s)
    lf = open(LOG, "ab")
    lf.write(b"===== boot =====\n" + boot + b"\n===== boot done =====\n")
    if not handshake(s):
        print("WARN: shell handshake timeout")
    for cmd in sys.argv[1:]:
        out = run_cmd(s, cmd, wait=120)
        extra = drain(s, 30)
        lf.write(b"\n>>> " + cmd.encode() + b"\n" + out + extra + b"\n")
        print(">>>", cmd)
        sys.stdout.write((out + extra).decode(errors="replace")[-4000:] + "\n")
    s.close()
    lf.close()
