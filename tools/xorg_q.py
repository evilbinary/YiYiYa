import os, socket, subprocess, sys, time
HOST, PORT = "127.0.0.1", 4460
KERNEL = "/Users/evil/dev/c/YiYiYa/build/raspi3/armv8-a/debug/kernel.elf"
# DISK=... 可覆盖：本机若已有另一个 QEMU 在用 image/disk.img（比如手动起的
# -serial stdio 实例），必须指到副本上，两个 QEMU 同写一个 raw 镜像会毁文件系统。
DISK = os.environ.get("DISK", "/Users/evil/dev/c/YiYiYa/image/disk.img")
LOG = "/tmp/xorg_qemu.log"
QEMU_CMD = ["qemu-system-aarch64", "-name", "YiYiYa-xorg", "-M", "raspi3b", "-rtc", "base=localtime", "-kernel", KERNEL, "-serial", "tcp:127.0.0.1:%d,server,nowait" % PORT, "-drive", "if=sd,id=sd0,format=raw,file=" + DISK]
import os
# 显示方式：none（默认，只看串口）/ cocoa（macOS 本机窗口）/ vnc（远程 VNC）
DISPLAY_MODE = os.environ.get("DISPLAY_MODE", "none")
if DISPLAY_MODE == "cocoa":
    # raspi3b 的显示设备是 SoC 里的 bcm2835-fb（内核通过 mailbox 申请显存），
    # 机器没有 PCI 总线 ⇒ 千万别加 -device VGA，否则 QEMU 直接报错退出。
    # 需要额外插 VGA（别的 machine）时才用 ADD_VGA=1 显式开。
    QEMU_CMD += ["-display", "cocoa"]
    if os.environ.get("ADD_VGA") == "1":
        QEMU_CMD += ["-device", "VGA"]
elif DISPLAY_MODE == "vnc":
    QEMU_CMD += ["-vnc", os.environ.get("VNC_ADDR", ":1")]
else:
    QEMU_CMD += ["-display", "none"]
if os.environ.get("GDB"):
    QEMU_CMD.insert(1, "-s")  # gdbstub on :1234

# MONITOR=/tmp/qmon.sock：开 HMP monitor 的 unix socket；SHOT=/tmp/x.ppm 在
# 脚本结束时做一次 screendump。
# 【实测结论】raspi3b + `-display none` 下 screendump 恒为全黑（1024x768 PPM），
# 但 guest 侧其实画对了：用 monitor 的 `xp /8wx 0x3c100000` 能看到 fb 里就是
# xsetroot 写的 0x00ff0000（红）。原因是 QEMU 的 bcm2835-fb 只有存在真实显示
# 后端（cocoa/vnc 且客户端在刷新）时才会把 vcram 脏行进 surface，headless 下
# 没人驱动刷新 → 截出来是黑的。要"看到"画面请用 DISPLAY_MODE=cocoa（或 vnc
# 并连一个客户端）；要"验证"画面请用 xp 读物理 0x3c100000。
MONITOR = os.environ.get("MONITOR")
if MONITOR:
    QEMU_CMD += ["-monitor", "unix:%s,server,nowait" % MONITOR]


def monitor_cmd(text, settle=1.5):
    """往 HMP monitor 发一条命令并读回输出（未开 MONITOR 时返回空串）。"""
    if not MONITOR:
        return ""
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    s.settimeout(3)
    try:
        s.connect(MONITOR)
    except OSError as e:
        return "monitor connect fail: %s" % e
    try:
        s.recv(65536)  # banner
    except OSError:
        pass
    s.sendall(text.encode() + b"\n")
    time.sleep(settle)
    out = b""
    try:
        out = s.recv(65536)
    except OSError:
        pass
    s.close()
    return out.decode(errors="replace")


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


def drain(s, secs=30, hard=90):
    """排空串口：空闲 secs 秒就结束，但【总时长】不超过 hard 秒。
    没有 hard 上限时，只要 guest 每 5 秒内吐一行日志（Xorg 的 dbus 重试、
    epoll 轮询都会），deadline 会被无限续命 ⇒ 后面的命令永远发不出去
    （实测表现为"卡在 Xorg 那一条，openbox/lxterminal 根本没执行"）。"""
    buf = bytearray()
    start = time.time()
    deadline = start + secs
    while time.time() < deadline and time.time() - start < hard:
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
    marker_wait = float(os.environ.get("MARK_WAIT", "60"))
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
        # 固定 40s 窗口常不够 ⇒ 用 WAIT/DRAIN 环境变量调
        wait = int(os.environ.get("WAIT", "40"))
        out = run_cmd(s, cmd, wait=wait)
        extra = drain(s, int(os.environ.get("DRAIN", "30")))
        lf.write(b"\n>>> " + cmd.encode() + b"\n" + out + extra + b"\n")
        lf.flush()  # MARKER 轮询读的是磁盘文件，必须先落盘
        print(">>>", cmd)
        sys.stdout.write((out + extra).decode(errors="replace")[-4000:] + "\n")
        # SLEEP=秒：额外等待一段时间（墙钟），期间持续泵串口。
        # 桌面这类"启动即常驻"的程序在 guest 里初始化很慢，而 init 的 shell
        # 不等子进程 ⇒ 必须靠这里给它时间，否则下一条命令会抢在初始化前面。
        # SLEEP_AFTER="2,3" 限定只在这些命令（1 基序号）之后等，
        # 否则像 rm 这种秒回的命令也要陪着等一整轮。
        slp = float(os.environ.get("SLEEP", "0"))
        picks = os.environ.get("SLEEP_AFTER")
        if picks is not None:
            try:
                wanted = {int(x) for x in picks.replace(" ", "").split(",") if x}
            except ValueError:
                wanted = set()
            if (i + 1) not in wanted:
                slp = 0.0
        end = time.time() + slp
        while slp > 0 and time.time() < end:
            try:
                d = s.recv(8192)
                if d:
                    lf.write(d)
                    lf.flush()
            except socket.timeout:
                pass
            time.sleep(0.2)
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
    shot = os.environ.get("SHOT")  # 例：SHOT=/tmp/desk.ppm
    if shot:
        print(monitor_cmd("screendump %s" % shot), flush=True)
    lf.close()
