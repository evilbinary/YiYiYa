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


def qemu_running():
    r = subprocess.run(["pgrep", "-f", "YiYiYa-xorg"], capture_output=True)
    return r.returncode == 0


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
    marker = os.environ.get("MARKER")  # 第一条命令后等日志出现该标记再继续
    marker_wait = float(os.environ.get("MARK_WAIT", "600"))
    reuse = os.environ.get("REUSE") == "1"
    if reuse and qemu_running():
        print("reuse running qemu")
    else:
        start_qemu()
    s = connect()
    boot = boot_wait(s)
    lf = open(LOG, "ab")
    lf.write(b"===== boot =====\n" + boot + b"\n===== boot done =====\n")
    if not handshake(s):
        print("WARN: shell handshake timeout")
    for i, cmd in enumerate(sys.argv[1:]):
        # guest 时钟远慢于墙钟（TCG 约 1s guest ≈ 80s wall），
        # 固定 120s 窗口常不够 ⇒ 用 WAIT/DRAIN 环境变量调
        wait = int(os.environ.get("WAIT", "120"))
        out = run_cmd(s, cmd, wait=wait)
        extra = drain(s, int(os.environ.get("DRAIN", "30")))
        lf.write(b"\n>>> " + cmd.encode() + b"\n" + out + extra + b"\n")
        lf.flush()  # MARKER 轮询读的是磁盘文件，必须先落盘
        print(">>>", cmd)
        sys.stdout.write((out + extra).decode(errors="replace")[-4000:] + "\n")
        if marker and i == 0:
            m = marker.encode()
            deadline = time.time() + marker_wait
            ok = False
            tail = bytearray()
            # 必须持续泵串口：不 recv 的话 QEMU 的 TCP 发送缓冲填满后
            # guest 串口写阻塞，整个 guest 会冻住；期间日志边收边落盘
            while time.time() < deadline:
                try:
                    d = s.recv(8192)
                    if d:
                        tail.extend(d)
                        lf.write(d)
                        lf.flush()
                        if m in bytes(tail):
                            ok = True
                            break
                except socket.timeout:
                    pass
                time.sleep(0.2)
            print("marker", marker, "found" if ok else "TIMEOUT", flush=True)
    s.close()
    lf.close()
