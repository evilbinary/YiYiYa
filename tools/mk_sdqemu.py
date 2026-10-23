#!/usr/bin/env python3
"""Build YiYiYa QEMU two-partition SD image (MBR: p1=FAT root, p2=data).

Usage: python3 tools/mk_sdqemu.py [p2.img] [out.img]
  p1 = image/disk.img (512MB FAT), p2 = given partition image (FAT32/ext4),
  total size rounded up to a power of 2 (QEMU SD-card requirement).
Defaults: p2=image/sda2.img out=image/sd_qemu.img
"""
import os
import sys

SEC = 512
P1_START = 2048
P1_SECS = 1048576
REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC_DISK = os.path.join(REPO, 'image/disk.img')
DEFAULT_P2 = os.path.join(REPO, 'image/sda2.img')
DEFAULT_OUT = os.path.join(REPO, 'image/sd_qemu.img')


def copy_at(src, dst, src_off, dst_off, nbytes):
    src.seek(src_off)
    dst.seek(dst_off)
    left = nbytes
    while left > 0:
        b = src.read(min(left, 1 << 22))
        if not b:
            break
        dst.write(b)
        left -= len(b)
    return nbytes - left


def mbr(p1_start, p1_secs, p2_start, p2_secs):
    m = bytearray(SEC)
    parts = [(p1_start, p1_secs, 0x0C, 0x80),
             (p2_start, p2_secs, 0x83, 0x00)]
    for idx, (start, n, ptype, boot) in enumerate(parts):
        o = 446 + idx * 16
        m[o] = boot
        for k in range(3):
            m[o + 1 + k] = (start >> (8 * k)) & 0xFF
        m[o + 4] = ptype
        for k in range(3):
            m[o + 5 + k] = 0xFE
        for k in range(4):
            m[o + 8 + k] = (start >> (8 * k)) & 0xFF
        for k in range(4):
            m[o + 12 + k] = (n >> (8 * k)) & 0xFF
    m[510] = 0x55
    m[511] = 0xAA
    return m


def pow2(n):
    v = 1
    while v < n:
        v <<= 1
    return v


def main():
    p2_path = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_P2
    out_path = sys.argv[2] if len(sys.argv) > 2 else DEFAULT_OUT
    p2_size = os.path.getsize(p2_path)
    p2_secs = (p2_size + SEC - 1) // SEC
    p2_start = P1_START + P1_SECS
    total_secs = pow2(p2_start + p2_secs)
    disk = open(SRC_DISK, 'rb')
    p2 = open(p2_path, 'rb')
    out = open(out_path, 'wb+')
    out.truncate(total_secs * SEC)
    out.seek(0)
    out.write(mbr(P1_START, P1_SECS, p2_start, p2_secs))
    copy_at(disk, out, 0, P1_START * SEC, P1_SECS * SEC)
    n2 = copy_at(p2, out, 0, p2_start * SEC, p2_size)
    out.close()
    print('p2=%dMB -> %s (%dMB, pow2)' %
          (n2 >> 20, out_path, total_secs * SEC >> 20))


if __name__ == '__main__':
    main()
