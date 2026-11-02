#!/usr/bin/env python3
"""V3D 7.1 控制流（BCL/RCL）解码器。

目的：不要再拿 hex 猜包。把 v3d_3d_min.c 的 dump_cl 输出（或任意 hex）按
mesa 的权威包头 `eggs/libmesa/src/broadcom/cle/v3d_packet_v71_pack.h` 解码成
「包名 + 长度 + 关键字段」，逐条和 mesa 的 emit 序列对照。

包名/长度/sub_id 表是运行时从 mesa 包头解析出来的，不走手抄，保证与上游
生成代码一致；字段级解码只覆盖本项目实际会发出的那些包。

用法：
    python3 tools/v3d_cl_decode.py --file /tmp/cl.log
    python3 tools/v3d_cl_decode.py --hex "79 00 80 02 e0 01 50 b0 01 ..."
    python3 tools/v3d_cl_decode.py --hex "..." --consumed 52
    (不给 --file/--hex 时从 stdin 读，自动挑 hex 行)

输入识别：以 "偏移:" 开头的 hex 行（dump_cl 格式），或任意纯 hex 文本。
"""

import argparse
import os
import re
import sys

PACK_HEADER = os.path.join(
    "eggs", "libmesa", "src", "broadcom", "cle", "v3d_packet_v71_pack.h"
)

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


# ---------------------------------------------------------------- 包头解析


def load_packet_table(path):
    """→ {(opcode, sub_id_or_None): (name, length)}"""
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        text = fh.read()

    opcode_of = {n: int(v) for n, v in re.findall(
        r"#define\s+V3D71_(\w+)_opcode\s+(\d+)", text)}
    length_of = {n: int(v) for n, v in re.findall(
        r"#define\s+V3D71_(\w+)_length\s+(\d+)", text)}

    sub_of = {}
    lines = text.splitlines()
    i = 0
    while i < len(lines):
        m = re.search(r"#define\s+V3D71_(\w+)_header\b", lines[i])
        if not m:
            i += 1
            continue
        name = m.group(1)
        body = [lines[i]]
        while body[-1].rstrip().endswith("\\") and i + 1 < len(lines):
            i += 1
            body.append(lines[i])
        sm = re.search(r"\.sub_id\s*=\s*(\d+)", "\n".join(body))
        if sm:
            sub_of[name] = int(sm.group(1))
        i += 1

    table = {}
    for name, op in opcode_of.items():
        length = length_of.get(name)
        if length is None:
            continue
        table[(op, sub_of.get(name))] = (name, length)
    return table


# ------------------------------------------------------------ 字段级解码


def _le(b, off, n):
    v = 0
    for k in range(n):
        v |= b[off + k] << (8 * k)
    return v


def _f32(b, off):
    import struct
    return struct.unpack("<f", bytes(b[off:off + 4]))[0]


def dec_common(b):
    h = ((b[7] >> 7) & 1) | ((b[8] & 3) << 1)
    w = (b[7] >> 4) & 7
    return ("%ux%u nrt=%u log2t=%dx%d early_z_dis=%d depth_dis=%d "
            "early_zs_clear=%d ez_dir=%d" % (
                _le(b, 2, 2), _le(b, 4, 2), ((b[1] >> 4) & 0xF) + 1,
                w, h, (b[6] >> 6) & 1, (b[6] >> 4) & 1,
                (b[7] >> 3) & 1, (b[6] >> 5) & 1))


def dec_rt_part1(b):
    base = (b[1] >> 7) | (b[2] << 1) | ((b[3] & 3) << 9)
    stride = ((b[3] >> 2) & 0x3F) | ((b[4] & 1) << 6)
    return ("rt=%d base=%d(512B) stride=%d bpp_enum=%d type_clamp=%d "
            "clear=0x%08x" % (
                (b[1] >> 3) & 7, base, stride + 1, (b[4] >> 1) & 3,
                (b[4] >> 3) & 0x1F, _le(b, 5, 4)))


def dec_zs_clear(b):
    return "z=%.6f stencil=0x%02x" % (_f32(b, 3), b[2])


def dec_binning_cfg(b):
    blk = {0: "64B", 1: "128B", 2: "256B"}
    return ("%ux%u log2t=%dx%d alloc_block=%s initial_block=%s" % (
        _le(b, 5, 2) + 1, _le(b, 7, 2) + 1, b[2] & 7, (b[2] >> 3) & 7,
        blk.get((b[1] >> 4) & 3, "?%d" % ((b[1] >> 4) & 3)),
        blk.get((b[1] >> 2) & 3, "?%d" % ((b[1] >> 2) & 3))))


def dec_tile_coords(b):
    return "col=%d row=%d" % (
        b[1] | ((b[2] & 0xF) << 8), ((b[2] >> 4) & 0xF) | (b[3] << 4))


def dec_supertile_coords(b):
    return "col=%d row=%d" % (b[1], b[2])


def _store_fields(b, fmt_key):
    buf = {0: "RT0", 1: "RT1", 2: "RT2", 3: "RT3", 4: "RT4", 5: "RT5",
           6: "RT6", 7: "RT7", 8: "NONE", 9: "Z", 10: "STENCIL",
           11: "ZSTENCIL"}
    mem = {0: "RASTER", 1: "LINEARTILE", 3: "UIF"}
    return ("buffer=%s mem_fmt=%s %s=%d rb_swap=%d clear_in_store=%d "
            "stride=%d height=%d addr=0x%08x" % (
                buf.get(b[1] & 0xF, "?%d" % (b[1] & 0xF)),
                mem.get((b[1] >> 4) & 7, "?%d" % ((b[1] >> 4) & 7)),
                fmt_key, (b[2] >> 4) | ((b[3] & 3) << 4),
                (b[3] >> 4) & 1, (b[3] >> 2) & 1,
                ((b[4] >> 4) & 0xF) | (b[5] << 4) | ((b[6] & 0xF) << 12),
                _le(b, 7, 2), _le(b, 9, 4)))


def dec_store(b):
    return _store_fields(b, "out_fmt")


def dec_load(b):
    return _store_fields(b, "in_fmt")


def dec_generic_list_addr(b):
    return "start=0x%08x end=0x%08x (len=%d)" % (
        _le(b, 1, 4), _le(b, 5, 4), _le(b, 5, 4) - _le(b, 1, 4))


def dec_supertile_cfg(b):
    return ("supertile=%dx%d frame_in_supertiles=%dx%d "
            "frame_in_tiles=%dx%d nblists=%d mc_en=%d" % (
                b[1] + 1, b[2] + 1, b[3], b[4],
                b[5] | ((b[6] & 0xF) << 8),
                ((b[6] >> 4) & 0xF) | (b[7] << 4),
                ((b[8] >> 5) & 7) + 1, b[8] & 1))


def dec_list_set_base(b):
    return "addr=0x%08x set=%d" % (_le(b, 1, 4) & ~0xF, b[1] & 0xF)


def dec_initial_block(b):
    blk = {0: "64B", 1: "128B", 2: "256B"}
    return "auto_chain=%d first_block=%s" % (
        (b[1] >> 2) & 1, blk.get(b[1] & 3, "?"))


def dec_layers(b):
    return "layers=%d" % (b[1] + 1)


def dec_shader_state(b):
    return "nattrs=%d ssr_addr=0x%08x" % (
        b[1] & 0x1F, (_le(b, 1, 4) & ~0x1F))


def dec_vertex_array(b):
    mode = {0: "POINTS", 1: "LINES", 2: "TRIANGLES"}
    return "mode=%s nverts=%d first=%d" % (
        mode.get(b[1], "?%d" % b[1]), _le(b, 2, 4), _le(b, 6, 4))


DECODERS = {
    "TILE_RENDERING_MODE_CFG_COMMON": dec_common,
    "TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART1": dec_rt_part1,
    "TILE_RENDERING_MODE_CFG_RENDER_TARGET_PART2": None,
    "TILE_RENDERING_MODE_CFG_ZS_CLEAR_VALUES": dec_zs_clear,
    "TILE_BINNING_MODE_CFG": dec_binning_cfg,
    "TILE_COORDINATES": dec_tile_coords,
    "SUPERTILE_COORDINATES": dec_supertile_coords,
    "STORE_TILE_BUFFER_GENERAL": dec_store,
    "LOAD_TILE_BUFFER_GENERAL": dec_load,
    "START_ADDRESS_OF_GENERIC_TILE_LIST": dec_generic_list_addr,
    "MULTICORE_RENDERING_SUPERTILE_CFG": dec_supertile_cfg,
    "MULTICORE_RENDERING_TILE_LIST_SET_BASE": dec_list_set_base,
    "TILE_LIST_INITIAL_BLOCK_SIZE": dec_initial_block,
    "NUMBER_OF_LAYERS": dec_layers,
    "GL_SHADER_STATE": dec_shader_state,
    "VERTEX_ARRAY_PRIMS": dec_vertex_array,
}


# ------------------------------------------------------------------ 输入


HEXLINE = re.compile(r"^\s*(?:v3d_3d:\s*)?(\d+):[> ]\s*((?:[0-9a-fA-F]{2}\s*)+)")


def read_bytes(path, hexarg):
    if hexarg:
        clean = re.sub(r"[^0-9a-fA-F]", "", hexarg)
        return bytes.fromhex(clean)

    text = open(path, "r", encoding="utf-8", errors="replace").read() \
        if path else sys.stdin.read()

    rows = {}
    for line in text.splitlines():
        m = HEXLINE.match(line)
        if m:
            rows[int(m.group(1))] = m.group(2).split()
    if rows:
        out = []
        for off in sorted(rows):
            out.extend(int(x, 16) for x in rows[off])
        return bytes(out)

    clean = re.sub(r"[^0-9a-fA-F]", "", text)
    if len(clean) % 2:
        clean = clean[:-1]
    return bytes.fromhex(clean)


# ------------------------------------------------------------------ 主流程


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--file", "--log", dest="path")
    ap.add_argument("--hex")
    ap.add_argument("--consumed", type=int, default=-1,
                    help="CT 的消费位置（相对偏移），标出停顿点")
    ap.add_argument("--packet-header", default=os.path.join(REPO_ROOT, PACK_HEADER))
    args = ap.parse_args()

    table = load_packet_table(args.packet_header)
    data = read_bytes(args.path, args.hex)
    if not data:
        sys.exit("no bytes parsed")

    multi = {}
    for (op, sub), (name, length) in table.items():
        multi.setdefault(op, []).append((sub, name, length))

    off = 0
    print("=== %d bytes, %d packets ===" % (len(data), 0))
    n = 0
    while off < len(data):
        op = data[off]
        cands = multi.get(op, [])
        name, length = "UNKNOWN", 1
        if len(cands) == 1:
            name, length = cands[0][1], cands[0][2]
        elif cands:
            sub = data[off + 1] & 7 if off + 1 < len(data) else 0
            for csub, cname, clen in cands:
                if csub == sub:
                    name, length = cname, clen
                    break
            else:
                name, length = cands[0][1], cands[0][2]

        b = data[off:off + length]
        if len(b) < length:
            print("%5d: %-46s (truncated, want %d got %d)"
                  % (off, name, length, len(b)))
            break

        mark = ""
        if args.consumed >= 0 and off <= args.consumed < off + length:
            mark = "  <== CT 停在这里"

        dec = DECODERS.get(name)
        detail = dec(b) if dec else ""
        print("%5d: %-46s len=%-3d hex=%s%s"
              % (off, name, length, bytes(b).hex(" "), mark))
        if detail:
            print("       %s" % detail)
        off += length
        n += 1

    print("=== %d packets decoded ===" % n)


if __name__ == "__main__":
    main()
