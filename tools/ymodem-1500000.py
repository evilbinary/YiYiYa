#!/usr/bin/env python3
# 给 U-Boot `loady` 发一个文件。macOS termios 设不到 1500000，和 serial-1500000.py 一样走 IOSSIOSPEED。
# 启动后先按键打断自动启动，停在 => 再 loady。传完执行 go，并留在这个串口里。退出：Ctrl-]
#   python3 tools/ymodem-1500000.py build/rk3568/armv8-a/debug/kernel.bin
import os
import select
import sys
import termios
import tty
import ctypes
import binascii

PORT = "/dev/cu.wchusbserial141400"
BAUD = 1500000
SOH = 0x01
STX = 0x02
EOT = 0x04
ACK = 0x06
NAK = 0x15
CRC = ord("C")


def set_baud(fd, baud):
    req = 0x80000000 | ((ctypes.sizeof(ctypes.c_ulong) & 0x1FFF) << 16) | (ord("T") << 8) | 2
    speed = ctypes.c_ulong(baud)
    rc = ctypes.CDLL(None).ioctl(fd, ctypes.c_ulong(req), ctypes.byref(speed))
    if rc != 0:
        raise OSError("set baud %d failed: %d" % (baud, rc))


class Rx:
    def __init__(self, fd):
        self.fd = fd
        self.buf = b""

    def take(self, want, seconds):
        import time
        deadline = time.time() + seconds
        while True:
            for i, b in enumerate(self.buf):
                if b in want:
                    self.buf = self.buf[i + 1:]
                    return b
            self.buf = b""
            left = deadline - time.time()
            if left <= 0:
                raise TimeoutError("waiting for %s" % [hex(b) for b in want])
            r, _, _ = select.select([self.fd], [], [], min(left, 1.0))
            if r:
                self.buf += os.read(self.fd, 4096)

    def drain(self, seconds):
        end = seconds
        while end > 0:
            r, _, _ = select.select([self.fd], [], [], end)
            if not r:
                break
            self.buf += os.read(self.fd, 4096)
            end = 0.05


def write_all(fd, data):
    off = 0
    while off < len(data):
        n = os.write(fd, data[off:])
        if n <= 0:
            raise OSError("serial write failed")
        off += n


def send_block(fd, rx, kind, seq, payload):
    size = 128 if kind == SOH else 1024
    data = payload[:size].ljust(size, b"\x1a")
    crc = binascii.crc_hqx(data, 0)
    pkt = bytes([kind, seq & 0xFF, (~seq) & 0xFF]) + data + bytes([(crc >> 8) & 0xFF, crc & 0xFF])
    # 1500000 上 CH340 会丢一字节。loady 丢包后重发的是 'C'，不是 NAK。
    # 旧逻辑只等 ACK/NAK，把 'C' 丢掉，干等 10 秒后失败。
    for _ in range(8):
        rx.buf = b""
        write_all(fd, pkt)
        try:
            got = rx.take((ACK, NAK, CRC), 2)
        except TimeoutError:
            continue
        if got == ACK:
            return
    raise TimeoutError("block %d no ACK" % seq)


def hold_uboot(fd, seconds=90):
    import time
    sys.stderr.write("stopping autoboot, waiting for =>\n")
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
        if b"yiyiya$" in buf:
            sys.stderr.write(
                "\n已经在 YiYiYa 里（yiyiya$），没有进入 U-Boot。\n"
                "请重启，在倒计时结束前再运行本脚本。\n"
            )
            return False
        tail = buf.rstrip(b" \t")
        if tail.endswith(b"=>"):
            return True
    sys.stderr.write("\n90 秒内没有看到 U-Boot 的 =>。\n")
    return False


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else None
    port = sys.argv[2] if len(sys.argv) > 2 else PORT
    if not path:
        sys.stderr.write("usage: %s <file> [port]\n" % sys.argv[0])
        return 1
    kick = "--loady" in sys.argv
    if kick:
        sys.argv.remove("--loady")
        path = sys.argv[1] if len(sys.argv) > 1 else None
        port = sys.argv[2] if len(sys.argv) > 2 else PORT
    blob = open(path, "rb").read()
    name = os.path.basename(path).encode("ascii", "replace")
    fd = os.open(port, os.O_RDWR | os.O_NOCTTY)
    attrs = termios.tcgetattr(fd)
    attrs[0] = 0
    attrs[1] = 0
    attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
    attrs[3] = 0
    attrs[6][termios.VMIN] = 0
    attrs[6][termios.VTIME] = 0
    # 上次 IOSSIOSPEED 留下的 1500000 不在 termios 波特率表里，
    # tcsetattr 会 EINVAL。先落到合法速率，再单独设 1500000。
    attrs[4] = termios.B9600
    attrs[5] = termios.B9600
    termios.tcsetattr(fd, termios.TCSANOW, attrs)
    set_baud(fd, BAUD)
    rx = Rx(fd)
    try:
        if not hold_uboot(fd):
            return 1
        os.write(fd, b"loady 0x02080000\r")
        sys.stderr.write("wait for loady on %s, %d bytes\n" % (port, len(blob)))
        rx.take((CRC,), 90)
        header = name + b"\0" + str(len(blob)).encode("ascii") + b" "
        send_block(fd, rx, SOH, 0, header)
        seq = 1
        off = 0
        while off < len(blob):
            send_block(fd, rx, STX, seq, blob[off:off + 1024])
            off += 1024
            seq = (seq + 1) & 0xFF
            sys.stderr.write("\r%d/%d" % (min(off, len(blob)), len(blob)))
        sys.stderr.write("\n")
        os.write(fd, bytes([EOT]))
        rx.take((ACK,), 10)
        rx.take((CRC, NAK), 10)
        send_block(fd, rx, SOH, 0, b"")
        rx.drain(0.4)
        if rx.buf:
            os.write(1, rx.buf)
        sys.stderr.write("\nsent %s\n" % path)
        write_all(fd, b"go 0x02080000\r")
        shell(fd)
    finally:
        os.close(fd)
    return 0


def shell(fd):
    stdin = sys.stdin.fileno()
    old = termios.tcgetattr(stdin)
    tty.setraw(stdin)
    sys.stderr.write("booting 0x02080000, shell on this port, quit with Ctrl-]\n")
    try:
        while True:
            r, _, _ = select.select([fd, stdin], [], [])
            if fd in r:
                data = os.read(fd, 4096)
                if data:
                    os.write(1, data)
            if stdin in r:
                key = os.read(stdin, 16)
                if not key or key == b"\x1d":
                    break
                os.write(fd, key)
    finally:
        termios.tcsetattr(stdin, termios.TCSANOW, old)
        sys.stderr.write("\nclosed\n")


if __name__ == "__main__":
    sys.exit(main())
