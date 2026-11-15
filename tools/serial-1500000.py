#!/usr/bin/env python3
# macOS 的 termios 设不到 1500000。这里用 IOSSIOSPEED 打开 CH340。
# 退出：Ctrl-]
import ctypes
import os
import select
import sys
import termios
import tty

PORT = "/dev/cu.wchusbserial141400"
BAUD = 1500000


def set_baud(fd, baud):
    # _IOW('T', 2, speed_t)，speed_t 在 64 位 macOS 上是 unsigned long
    req = 0x80000000 | ((ctypes.sizeof(ctypes.c_ulong) & 0x1FFF) << 16) | (ord("T") << 8) | 2
    speed = ctypes.c_ulong(baud)
    rc = ctypes.CDLL(None).ioctl(fd, ctypes.c_ulong(req), ctypes.byref(speed))
    if rc != 0:
        raise OSError("set baud %d failed: %d" % (baud, rc))


def main():
    port = sys.argv[1] if len(sys.argv) > 1 else PORT
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

    stdin = sys.stdin.fileno()
    old = termios.tcgetattr(stdin)
    tty.setraw(stdin)
    sys.stderr.write("open %s %d 8N1, quit with Ctrl-]\n" % (port, BAUD))
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
        os.close(fd)
        sys.stderr.write("\nclosed\n")


if __name__ == "__main__":
    main()
