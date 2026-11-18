#!/usr/bin/env python3
# 把本机文件经 YMODEM 写进 RK3568 的 FAT 启动分区。
# 板子要停在 U-Boot 的 =>。长度按十六进制传给 fatwrite。
#
#   python3 tools/fat-put.py bin/logo=app/resource/bin/logo bin/devd=app/resource/bin/devd
#   python3 tools/fat-put.py --init app/resource/conf/init.conf --root app/resource/bin \
#       --boot build/rk3568/armv8-a/debug/kernel.bin
#
# --init 读取未注释的 /bin/名字，到 --root 里找同名文件。找不到的跳过并打印。
import importlib.util
import os
import select
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
spec = importlib.util.spec_from_file_location(
    "ym", os.path.join(ROOT, "tools", "ymodem-1500000.py"))
ym = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ym)

LOAD_ADDR = 0x04000000


def wait_uboot(fd, seconds):
    sys.stderr.write("waiting for U-Boot =>\n")
    sys.stderr.flush()
    buf = b""
    deadline = time.time() + seconds
    next_key = 0
    while time.time() < deadline:
        now = time.time()
        if now >= next_key:
            os.write(fd, b"\r")
            next_key = now + 0.15
        r, _, _ = select.select([fd], [], [], 0.05)
        if not r:
            continue
        data = os.read(fd, 4096)
        if not data:
            continue
        os.write(1, data)
        buf = (buf + data)[-4096:]
        tail = buf.rstrip(b" \t\r\n")
        if tail.endswith(b"=>"):
            return True
    sys.stderr.write("no U-Boot =>\n")
    return False


def wait_prompt(fd, rx, seconds):
    deadline = time.time() + seconds
    buf = b""
    while time.time() < deadline:
        r, _, _ = select.select([fd], [], [], 0.2)
        if not r:
            continue
        data = os.read(fd, 4096)
        if not data:
            continue
        os.write(1, data)
        buf = (buf + data)[-8192:]
        rx.buf += data
        if buf.rstrip(b" \t\r\n").endswith(b"=>"):
            rx.buf = b""
            return True
    return False


def uboot(fd, rx, line, seconds):
    sys.stderr.write(">> %s\n" % line)
    sys.stderr.flush()
    rx.buf = b""
    ym.write_all(fd, (line + "\r").encode("ascii"))
    if not wait_prompt(fd, rx, seconds):
        raise TimeoutError("no prompt after " + line)


def send_file(fd, rx, path, addr):
    blob = open(path, "rb").read()
    name = os.path.basename(path).encode("ascii", "replace")
    ym.write_all(fd, ("loady 0x%x\r" % addr).encode("ascii"))
    sys.stderr.write("loady 0x%x %d bytes %s\n" % (addr, len(blob), path))
    sys.stderr.flush()
    rx.buf = b""
    rx.take((ym.CRC,), 90)
    header = name + b"\0" + str(len(blob)).encode("ascii") + b" "
    ym.send_block(fd, rx, ym.SOH, 0, header)
    seq = 1
    off = 0
    while off < len(blob):
        ym.send_block(fd, rx, ym.STX, seq, blob[off:off + 1024])
        off += 1024
        seq = (seq + 1) & 0xFF
        if off >= len(blob) or (off & 0xFFFF) == 0:
            sys.stderr.write("\r%d/%d" % (min(off, len(blob)), len(blob)))
            sys.stderr.flush()
    sys.stderr.write("\n")
    os.write(fd, bytes([ym.EOT]))
    rx.take((ym.ACK,), 10)
    rx.take((ym.CRC, ym.NAK), 10)
    ym.send_block(fd, rx, ym.SOH, 0, b"")
    rx.drain(0.4)
    if rx.buf:
        os.write(1, rx.buf)
        rx.buf = b""
    return len(blob)


def open_port():
    port = os.environ.get("FAT_PORT", ym.PORT)
    sys.stderr.write("serial port %s\n" % port)
    fd = os.open(port, os.O_RDWR | os.O_NOCTTY)
    attrs = ym.termios.tcgetattr(fd)
    attrs[0] = 0
    attrs[1] = 0
    attrs[2] = ym.termios.CS8 | ym.termios.CREAD | ym.termios.CLOCAL
    attrs[3] = 0
    attrs[6][ym.termios.VMIN] = 0
    attrs[6][ym.termios.VTIME] = 0
    attrs[4] = ym.termios.B9600
    attrs[5] = ym.termios.B9600
    ym.termios.tcsetattr(fd, ym.termios.TCSANOW, attrs)
    ym.set_baud(fd, ym.BAUD)
    return fd


def from_init(conf, root):
    pairs = []
    missing = []
    for raw in open(conf, "r", encoding="utf-8", errors="replace"):
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        if line.startswith("wait "):
            line = line[5:].strip()
        token = line.split()[0]
        if not token.startswith("/bin/"):
            continue
        name = token[5:]
        src = os.path.join(root, name)
        if not os.path.isfile(src):
            missing.append(name)
            continue
        pairs.append(("bin/" + name, src))
    return pairs, missing


def parse_args(argv):
    boot = None
    conf = None
    root = os.path.join(ROOT, "app", "resource", "bin")
    pairs = []
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--boot":
            i += 1
            boot = argv[i]
        elif a == "--init":
            i += 1
            conf = argv[i]
        elif a == "--root":
            i += 1
            root = argv[i]
        elif "=" in a:
            dest, src = a.split("=", 1)
            pairs.append((dest, src))
        else:
            sys.stderr.write("usage: fat-put.py [--init conf --root dir] [--boot kernel] dest=src ...\n")
            return None
        i += 1
    if conf:
        found, missing = from_init(conf, root)
        for name in missing:
            sys.stderr.write("skip %s: not in %s\n" % (name, root))
        pairs = found + pairs
    return pairs, boot


def main():
    parsed = parse_args(sys.argv[1:])
    if not parsed:
        return 1
    pairs, boot = parsed
    if not pairs and not boot:
        sys.stderr.write("nothing to write\n")
        return 1
    fd = open_port()
    rx = ym.Rx(fd)
    made = set()
    try:
        if not wait_uboot(fd, 30):
            return 1
        uboot(fd, rx, "mmc dev 0", 10)
        for dest, src in pairs:
            if not os.path.isfile(src):
                raise OSError("missing " + src)
            parent = dest.rsplit("/", 1)[0] if "/" in dest else ""
            if parent:
                acc = ""
                for part in parent.split("/"):
                    acc = part if not acc else acc + "/" + part
                    if acc in made:
                        continue
                    uboot(fd, rx, "fatmkdir mmc 0:1 " + acc, 5)
                    made.add(acc)
            n = send_file(fd, rx, src, LOAD_ADDR)
            uboot(fd, rx, "fatwrite mmc 0:1 0x%x %s 0x%x" % (LOAD_ADDR, dest, n), 60)
            sys.stderr.write("wrote %s (%d)\n" % (dest, n))
        if pairs:
            uboot(fd, rx, "fatls mmc 0:1 bin", 10)
        if boot:
            send_file(fd, rx, boot, 0x02080000)
            ym.write_all(fd, b"go 0x02080000\r")
            sys.stderr.write("kernel started\n")
            end = time.time() + 20
            while time.time() < end:
                r, _, _ = select.select([fd], [], [], 0.5)
                if r:
                    data = os.read(fd, 4096)
                    if data:
                        os.write(1, data)
        return 0
    finally:
        os.close(fd)


if __name__ == "__main__":
    sys.exit(main())
