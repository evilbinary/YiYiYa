#!/usr/bin/env python3
"""把 fbshot 生成的 P6 PPM 转成 PNG（只用标准库）。"""
import struct
import sys
import zlib


def read_ppm(path):
    with open(path, "rb") as f:
        tokens = []
        while len(tokens) < 4:
            line = f.readline()
            if not line:
                raise ValueError("truncated PPM header")
            if line.startswith(b"#"):
                continue
            tokens += line.split()
        if tokens[0] != b"P6":
            raise ValueError("not a P6 PPM")
        w = int(tokens[1])
        h = int(tokens[2])
        maxv = int(tokens[3])
        if maxv != 255:
            raise ValueError("only maxval=255 supported")
        data = f.read(w * h * 3)
    if len(data) != w * h * 3:
        raise ValueError("short PPM data")
    return w, h, data


def write_png(path, w, h, rgb):
    stride = w * 3
    raw = bytearray()
    for y in range(h):
        raw.append(0)  # filter: none
        raw += rgb[y * stride:(y + 1) * stride]

    def chunk(tag, data):
        body = tag + data
        return (struct.pack(">I", len(data)) + body +
                struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF))

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(bytes(raw), 6)))
        f.write(chunk(b"IEND", b""))


def main():
    if len(sys.argv) != 3:
        print("usage: ppm2png.py input.ppm output.png", file=sys.stderr)
        return 2
    w, h, rgb = read_ppm(sys.argv[1])
    write_png(sys.argv[2], w, h, rgb)
    print(f"{sys.argv[2]} {w}x{h}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
