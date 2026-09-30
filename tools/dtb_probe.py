#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""从真机 DTB 里挖 RP1 UART 的 CPU 侧地址（以及相关 ranges/时钟）。

背景：Pi5 的 UART0 在 RP1 里、RP1 是 PCIe 设备，CPU 侧有多个窗口，
项目里 5 个候选地址实测都打不出字 ⇒ 需要 DTB 里的权威答案。

用法：
    python3 tools/dtb_probe.py /Volumes/bootfs/bcm2712-rpi-5-b.dtb

输出：chosen/stdout-path、所有名字含 serial/rp1/pcie/clocks 的节点的
      reg / ranges / compatible / clock-frequency 等属性（原始值 + 十六进制）。
"""
import struct
import sys

FDT_MAGIC = 0xD00DFEED
FDT_BEGIN_NODE, FDT_END_NODE, FDT_PROP, FDT_NOP, FDT_END = 1, 2, 3, 4, 9


def read_dtb(path):
    data = open(path, 'rb').read()
    magic, totalsize, off_struct, off_strings, off_rsvmap, version, \
        last_comp, boot_cpu, size_strings, size_struct = struct.unpack_from('>10I', data, 0)
    if magic != FDT_MAGIC:
        raise SystemExit('不是 DTB 文件（magic=%#x）' % magic)
    return data, off_struct, off_strings, size_struct


def parse(data, off_struct, off_strings, size_struct):
    """返回 [(path, name, props)]，props = {name: bytes}"""
    out = []
    pos = off_struct
    end = off_struct + size_struct
    stack = []  # [(name, props)]，props 必须每层各一个（子节点不能覆盖父节点的）
    while pos < end:
        tok, = struct.unpack_from('>I', data, pos)
        pos += 4
        if tok == FDT_BEGIN_NODE:
            name_end = data.index(b'\0', pos)
            name = data[pos:name_end].decode('utf-8', 'replace')
            pos = (name_end + 4) & ~3
            stack.append([name or '/', {}])
        elif tok == FDT_END_NODE:
            name, props = stack.pop()
            names = [n[0] for n in stack] + [name]
            path = '/' + '/'.join(x for x in names if x and x != '/')
            out.append((path, name, props))
        elif tok == FDT_PROP:
            length, nameoff = struct.unpack_from('>II', data, pos)
            pos += 8
            val = data[pos:pos + length]
            pos = (pos + length + 3) & ~3
            nend = data.index(b'\0', off_strings + nameoff)
            pname = data[off_strings + nameoff:nend].decode('utf-8', 'replace')
            if stack:
                stack[-1][1][pname] = val
        elif tok == FDT_NOP:
            pass
        elif tok == FDT_END:
            break
    return out


def show_cells(val):
    if len(val) % 4:
        return repr(val)
    return ' '.join('%#x' % c for c in struct.unpack('>%dI' % (len(val) // 4), val))


INTEREST = ('serial', 'uart', 'rp1', 'pcie', 'clocks', 'clock-controller')
PROPS = ('compatible', 'reg', 'ranges', 'clock-frequency', 'status',
         'stdout-path', 'clock-names', 'assigned-clock-rates')


def main():
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    data, off_struct, off_strings, size_struct = read_dtb(sys.argv[1])
    nodes = parse(data, off_struct, off_strings, size_struct)
    print('DTB 共 %d 个节点\n' % len(nodes))
    for path, name, props in nodes:
        low = (path + '/' + name).lower()
        if 'chosen' in low or any(k in low for k in INTEREST):
            print('=== %s ===' % path)
            for p in PROPS:
                if p in props:
                    v = props[p]
                    if p == 'compatible':
                        print('    %-18s %s' % (p, v.split(b'\0')[0].decode('utf-8', 'replace')))
                    elif p == 'status':
                        print('    %-18s %s' % (p, v.split(b'\0')[0].decode('utf-8', 'replace')))
                    elif p == 'stdout-path':
                        print('    %-18s %s' % (p, v.split(b'\0')[0].decode('utf-8', 'replace')))
                    else:
                        print('    %-18s %s' % (p, show_cells(v)))
            print()


if __name__ == '__main__':
    main()
