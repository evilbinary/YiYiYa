#!/usr/bin/env python3
"""用 py-videocore7 的汇编器生成 YiYiYa 的 QPU 内核。

为什么用它：
  1. 我们自己手工 pack 的 `nop` 是 0x38000000bb03f000，
     而 py-videocore7（真机验证过）产出 0x38003186bb03f000 ——
     低 16 位的 0x3186 我们一直是 0，循环因此从不生效。
  2. b 操作数的整数小立即数只支持 0..15 / -16..-1，
     所以 `+= 64`、`& 0xFF`、`>> 24` 这些都必须用寄存器承载。

用法：
    python3 vc7_gen.py > /tmp/k.h
    然后把 /tmp/k.h 里的两个数组并入
    foot/driver/gpu/v3d/v3d_qpu_kernels.h 的末尾（在 #endif 之前）。

注意：内核已经并入 v3d_qpu_kernels.h，独立的 two_d.h 已删除。
重新生成后要手工替换 kernels.h 里那一段，否则会保留旧内核。
"""
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)

from vc7.assembler import Assembly, qpu, assemble  # noqa: E402


@qpu
def qpu_blit(asm: Assembly, *, rounds: int) -> None:
    """16 lane 并行拷贝，每轮 64 字节。
    uniform: u0=src u1=dst u2=rounds u3=64
    """
    reg_src = rf[47]
    reg_dst = rf[48]
    reg_cnt = rf[49]
    reg_adv = rf[26]
    reg_qs = rf[25]      # 每个 QPU 负责的字节跨度
    reg_q = rf[24]       # QPU 号

    nop(sig=ldunifrf(reg_src))   # noqa: F821
    nop(sig=ldunifrf(reg_dst))   # noqa: F821
    nop(sig=ldunifrf(reg_cnt))   # noqa: F821
    nop(sig=ldunifrf(reg_adv))   # noqa: F821
    nop(sig=ldunifrf(reg_qs))    # noqa: F821
    nop()                        # noqa: F821
    nop()                        # noqa: F821
    nop()                        # noqa: F821
    nop()                        # noqa: F821

    # 每个 QPU 处理连续的一段：起始偏移 = QPU号 × 跨度
    tidx(reg_q)                                      # noqa: F821
    shr(reg_q, reg_q, 2)                             # noqa: F821  QPU 号
    umul24(reg_q, reg_q, reg_qs)                     # noqa: F821
    add(reg_src, reg_src, reg_q)                     # noqa: F821
    add(reg_dst, reg_dst, reg_q)                     # noqa: F821

    with loop as l:                                  # noqa: F821
        eidx(rf[20])                                 # noqa: F821
        shl(rf[21], rf[20], 2)                       # noqa: F821
        add(rf[22], reg_src, rf[21])                 # noqa: F821
        mov(tmua, rf[22])                            # noqa: F821
        nop()                                        # noqa: F821
        nop()                                        # noqa: F821
        nop(sig=ldtmu(rf[28]))                       # noqa: F821
        add(rf[23], reg_dst, rf[21])                 # noqa: F821
        mov(tmud, rf[28])                            # noqa: F821
        mov(tmua, rf[23])                            # noqa: F821
        # 等 TMU 写排空再进下一轮：每轮 16 读 + 16 写正好顶到输入 FIFO
        # 的 16 格上限，不排空会偶发丢请求（实测约 4% 的像素错，均匀分布）。
        tmuwt()                                      # noqa: F821
        add(reg_src, reg_src, reg_adv)               # noqa: F821
        add(reg_dst, reg_dst, reg_adv)               # noqa: F821
        sub(reg_cnt, reg_cnt, 1, cond="pushz")       # noqa: F821
        l.b(cond="na0").unif_addr(absolute=False)    # noqa: F821
        nop()                                        # noqa: F821
        nop()                                        # noqa: F821
        nop()                                        # noqa: F821

    nop()                        # noqa: F821
    nop(sig=thrsw)               # noqa: F821
    nop()                        # noqa: F821
    nop()                        # noqa: F821


@qpu
def qpu_alpha(asm: Assembly, *, rounds: int) -> None:
    """16 lane 并行的 ARGB8888 source-over 混合。

    dst = (src*a + dst*(256-a)) >> 8     （逐通道，a = 0..256）

    uniform: u0=src u1=dst u2=rounds u3=64
             u4=a u5=256-a u6=0xFF u7=8 u8=16 u9=24
    """
    reg_src = rf[47]
    reg_dst = rf[48]
    reg_cnt = rf[49]
    reg_adv = rf[26]
    reg_qs = rf[35]
    reg_q = rf[34]
    reg_a = rf[45]
    reg_inv = rf[44]
    reg_ff = rf[43]
    reg_s8 = rf[42]
    reg_s16 = rf[41]
    reg_s24 = rf[40]

    nop(sig=ldunifrf(reg_src))   # noqa: F821
    nop(sig=ldunifrf(reg_dst))   # noqa: F821
    nop(sig=ldunifrf(reg_cnt))   # noqa: F821
    nop(sig=ldunifrf(reg_adv))   # noqa: F821
    nop(sig=ldunifrf(reg_a))     # noqa: F821
    nop(sig=ldunifrf(reg_inv))   # noqa: F821
    nop(sig=ldunifrf(reg_ff))    # noqa: F821
    nop(sig=ldunifrf(reg_s8))    # noqa: F821
    nop(sig=ldunifrf(reg_s16))   # noqa: F821
    nop(sig=ldunifrf(reg_s24))   # noqa: F821
    nop(sig=ldunifrf(reg_qs))    # noqa: F821
    nop()                        # noqa: F821
    nop()                        # noqa: F821
    nop()                        # noqa: F821

    tidx(reg_q)                                      # noqa: F821
    shr(reg_q, reg_q, 2)                             # noqa: F821
    umul24(reg_q, reg_q, reg_qs)                     # noqa: F821
    add(reg_src, reg_src, reg_q)                     # noqa: F821
    add(reg_dst, reg_dst, reg_q)                     # noqa: F821

    with loop as l:                                  # noqa: F821
        eidx(rf[20])                                 # noqa: F821
        shl(rf[21], rf[20], 2)                       # noqa: F821

        # ---- 读 src ----
        add(rf[22], reg_src, rf[21])                 # noqa: F821
        mov(tmua, rf[22])                            # noqa: F821
        nop()                                        # noqa: F821
        nop()                                        # noqa: F821
        nop(sig=ldtmu(rf[28]))                       # noqa: F821

        # ---- 读 dst ----
        add(rf[27], reg_dst, rf[21])                 # noqa: F821
        mov(tmua, rf[27])                            # noqa: F821
        nop()                                        # noqa: F821
        nop()                                        # noqa: F821
        nop(sig=ldtmu(rf[29]))                       # noqa: F821

        # ---- 逐通道混合 ----
        # rf16 = 结果；通道顺序 B(0) G(8) R(16) A(24)
        mov(rf[16], rf[29])                          # noqa: F821  先放 dst 的 B
        band(rf[16], rf[16], reg_ff)                  # noqa: F821
        umul24(rf[17], rf[16], reg_inv)              # noqa: F821
        band(rf[18], rf[28], reg_ff)                  # noqa: F821
        umul24(rf[18], rf[18], reg_a)                # noqa: F821
        add(rf[17], rf[17], rf[18])                  # noqa: F821
        shr(rf[17], rf[17], reg_s8)                  # noqa: F821
        mov(rf[16], rf[17])                          # noqa: F821

        # G 通道
        shr(rf[19], rf[29], reg_s8)                  # noqa: F821
        band(rf[19], rf[19], reg_ff)                  # noqa: F821
        umul24(rf[17], rf[19], reg_inv)              # noqa: F821
        shr(rf[18], rf[28], reg_s8)                  # noqa: F821
        band(rf[18], rf[18], reg_ff)                  # noqa: F821
        umul24(rf[18], rf[18], reg_a)                # noqa: F821
        add(rf[17], rf[17], rf[18])                  # noqa: F821
        shr(rf[17], rf[17], reg_s8)                  # noqa: F821
        shl(rf[17], rf[17], reg_s8)                  # noqa: F821
        bor(rf[16], rf[16], rf[17])                  # noqa: F821

        # R 通道
        shr(rf[19], rf[29], reg_s16)                 # noqa: F821
        band(rf[19], rf[19], reg_ff)                  # noqa: F821
        umul24(rf[17], rf[19], reg_inv)              # noqa: F821
        shr(rf[18], rf[28], reg_s16)                 # noqa: F821
        band(rf[18], rf[18], reg_ff)                  # noqa: F821
        umul24(rf[18], rf[18], reg_a)                # noqa: F821
        add(rf[17], rf[17], rf[18])                  # noqa: F821
        shr(rf[17], rf[17], reg_s8)                  # noqa: F821
        shl(rf[17], rf[17], reg_s16)                 # noqa: F821
        bor(rf[16], rf[16], rf[17])                  # noqa: F821

        # A 通道
        shr(rf[19], rf[29], reg_s24)                 # noqa: F821
        umul24(rf[17], rf[19], reg_inv)              # noqa: F821
        shr(rf[18], rf[28], reg_s24)                 # noqa: F821
        umul24(rf[18], rf[18], reg_a)                # noqa: F821
        add(rf[17], rf[17], rf[18])                  # noqa: F821
        shr(rf[17], rf[17], reg_s8)                  # noqa: F821
        shl(rf[17], rf[17], reg_s24)                 # noqa: F821
        bor(rf[16], rf[16], rf[17])                  # noqa: F821

        # ---- 写回 ----
        mov(tmud, rf[16])                            # noqa: F821
        mov(tmua, rf[27])                            # noqa: F821
        tmuwt()                                      # noqa: F821
        add(reg_src, reg_src, reg_adv)               # noqa: F821
        add(reg_dst, reg_dst, reg_adv)               # noqa: F821
        sub(reg_cnt, reg_cnt, 1, cond="pushz")       # noqa: F821
        l.b(cond="na0").unif_addr(absolute=False)    # noqa: F821
        nop()                                        # noqa: F821
        nop()                                        # noqa: F821
        nop()                                        # noqa: F821

    nop()                        # noqa: F821
    nop(sig=thrsw)               # noqa: F821
    nop()                        # noqa: F821
    nop()                        # noqa: F821


@qpu
def qpu_present(asm: Assembly, *, rounds: int) -> None:
    """ARGB8888 -> RGB565 打包上屏（12 QPU 分段，与 copy_vec 同构）。

    每 lane 处理 2 个源像素（8 字节）→ 输出 1 个 32 位字（2 个 RGB565）。
    必须凑 2 个像素：TMU 写是 32 位的，单个 16 位像素写不出去。

    ── 寄存器只有三个可用位置 ──
    b 操作数必须 < 32；0..24 大多是 magic / 7.x 保留段；28..31 是 ldtmu 的
    目标区间（写进去会被 TMU 覆盖）。真正干净的只有 25/26/27：

        rf25 = qpu_stride   每个 QPU 负责的字节跨度
        rf26 = 128          每轮源步长（16 lane × 8 字节）
        rf27 = 64           每轮目的步长（16 lane × 4 字节）

    **所以掩码 0x1F/0x3F 一个都放不下**，改成纯移位取位：
        (x << 27) >> 27  ==  x & 0x1F     （27 = 15 + 12 两次移位）

    ── 转换 ──
        B = ((c >> 3)  & 0x1F)
        G = (((c >> 10) & 0x3F) << 5)
        R = (((c >> 19) & 0x1F) << 11)    19 = 15 + 4

    uniform: u0=src u1=dst u2=每QPU轮数 u3=128 u4=64 u5=qpu_stride
    """
    reg_src = rf[47]
    reg_dst = rf[48]
    reg_cnt = rf[49]
    reg_sadv = rf[26]
    reg_dadv = rf[27]
    reg_qs = rf[25]
    reg_idx = rf[50]
    reg_q = rf[58]
    reg_tmp = rf[51]
    reg_c0 = rf[52]
    reg_t = rf[53]
    reg_c1 = rf[54]
    reg_out = rf[56]

    nop(sig=ldunifrf(reg_src))     # noqa: F821
    nop(sig=ldunifrf(reg_dst))     # noqa: F821
    nop(sig=ldunifrf(reg_cnt))     # noqa: F821
    nop(sig=ldunifrf(reg_sadv))    # noqa: F821
    nop(sig=ldunifrf(reg_dadv))    # noqa: F821
    nop(sig=ldunifrf(reg_qs))      # noqa: F821
    nop()                          # noqa: F821
    nop()                          # noqa: F821
    nop()                          # noqa: F821
    nop()                          # noqa: F821

    # 每个 QPU 认领连续的一段
    tidx(reg_q)                                    # noqa: F821
    shr(reg_q, reg_q, 2)                           # noqa: F821
    umul24(reg_q, reg_q, reg_qs)                   # noqa: F821
    add(reg_src, reg_src, reg_q)                   # noqa: F821
    add(reg_dst, reg_dst, reg_q)                   # noqa: F821

    with loop as l:                                  # noqa: F821
        eidx(reg_idx)                                # noqa: F821
        shl(reg_idx, reg_idx, 3)                     # noqa: F821  lane*8

        # ======== 第一个像素 -> c0（低 16 位）========
        add(reg_tmp, reg_src, reg_idx)               # noqa: F821
        mov(tmua, reg_tmp)                           # noqa: F821
        nop()                                        # noqa: F821
        nop()                                        # noqa: F821
        nop(sig=ldtmu(rf[28]))                       # noqa: F821

        # B = (c >> 3) & 0x1F ：用 <<27 >>27 取低 5 位（省掉掩码寄存器）
        shr(reg_c0, rf[28], 3)                       # noqa: F821
        shl(reg_c0, reg_c0, 15)                      # noqa: F821
        shl(reg_c0, reg_c0, 12)                      # noqa: F821
        shr(reg_c0, reg_c0, 15)                      # noqa: F821
        shr(reg_c0, reg_c0, 12)                      # noqa: F821
        # G = ((c >> 10) & 0x3F) << 5
        shr(reg_t, rf[28], 10)                       # noqa: F821
        shl(reg_t, reg_t, 15)                        # noqa: F821
        shl(reg_t, reg_t, 11)                        # noqa: F821
        shr(reg_t, reg_t, 15)                        # noqa: F821
        shr(reg_t, reg_t, 11)                        # noqa: F821
        shl(reg_t, reg_t, 5)                         # noqa: F821
        bor(reg_c0, reg_c0, reg_t)                   # noqa: F821
        # R = ((c >> 19) & 0x1F) << 11   （19 = 15 + 4）
        shr(reg_t, rf[28], 15)                       # noqa: F821
        shr(reg_t, reg_t, 4)                         # noqa: F821
        shl(reg_t, reg_t, 15)                        # noqa: F821
        shl(reg_t, reg_t, 12)                        # noqa: F821
        shr(reg_t, reg_t, 15)                        # noqa: F821
        shr(reg_t, reg_t, 12)                        # noqa: F821
        shl(reg_t, reg_t, 11)                        # noqa: F821
        bor(reg_c0, reg_c0, reg_t)                   # noqa: F821

        # ======== 第二个像素 -> c1 ========
        add(reg_tmp, reg_tmp, 4)                     # noqa: F821
        mov(tmua, reg_tmp)                           # noqa: F821
        nop()                                        # noqa: F821
        nop()                                        # noqa: F821
        nop(sig=ldtmu(rf[29]))                       # noqa: F821

        shr(reg_c1, rf[29], 3)                       # noqa: F821
        shl(reg_c1, reg_c1, 15)                      # noqa: F821
        shl(reg_c1, reg_c1, 12)                      # noqa: F821
        shr(reg_c1, reg_c1, 15)                      # noqa: F821
        shr(reg_c1, reg_c1, 12)                      # noqa: F821
        shr(reg_t, rf[29], 10)                       # noqa: F821
        shl(reg_t, reg_t, 15)                        # noqa: F821
        shl(reg_t, reg_t, 11)                        # noqa: F821
        shr(reg_t, reg_t, 15)                        # noqa: F821
        shr(reg_t, reg_t, 11)                        # noqa: F821
        shl(reg_t, reg_t, 5)                         # noqa: F821
        bor(reg_c1, reg_c1, reg_t)                   # noqa: F821
        shr(reg_t, rf[29], 15)                       # noqa: F821
        shr(reg_t, reg_t, 4)                         # noqa: F821
        shl(reg_t, reg_t, 15)                        # noqa: F821
        shl(reg_t, reg_t, 12)                        # noqa: F821
        shr(reg_t, reg_t, 15)                        # noqa: F821
        shr(reg_t, reg_t, 12)                        # noqa: F821
        shl(reg_t, reg_t, 11)                        # noqa: F821
        bor(reg_c1, reg_c1, reg_t)                   # noqa: F821

        # 拼成一个 32 位字：高 16 = c1，低 16 = c0
        shl(reg_c1, reg_c1, 8)                       # noqa: F821
        shl(reg_c1, reg_c1, 8)                       # noqa: F821
        bor(reg_out, reg_c1, reg_c0)                 # noqa: F821

        # 写进紧凑的 scratch：目的偏移 = lane*4
        shr(reg_t, reg_idx, 1)                       # noqa: F821
        add(reg_t, reg_dst, reg_t)                   # noqa: F821
        mov(tmud, reg_out)                           # noqa: F821
        mov(tmua, reg_t)                             # noqa: F821
        tmuwt()                                      # noqa: F821

        add(reg_src, reg_src, reg_sadv)              # noqa: F821
        add(reg_dst, reg_dst, reg_dadv)              # noqa: F821
        sub(reg_cnt, reg_cnt, 1, cond="pushz")       # noqa: F821
        l.b(cond="na0").unif_addr(absolute=False)    # noqa: F821
        nop()                                        # noqa: F821
        nop()                                        # noqa: F821
        nop()                                        # noqa: F821

    nop()                        # noqa: F821
    nop(sig=thrsw)               # noqa: F821
    nop()                        # noqa: F821
    nop()                        # noqa: F821


def emit(name, code):
    out = ['static const uint64_t %s[] = {' % name]
    for w in code:
        out.append('    0x%016xULL,' % w)
    out.append('};')
    out.append('static const unsigned %s_n = %d;' % (name, len(code)))
    return '\n'.join(out)


def main():
    blit = assemble(qpu_blit, rounds=8)
    alpha = assemble(qpu_alpha, rounds=8)
    present = assemble(qpu_present, rounds=8)

    print('/* 由 tools/qpudis/vc7_gen.py 用 py-videocore7 汇编器生成，勿手改 */')
    print('#ifndef G2D_TWO_D_H')
    print('#define G2D_TWO_D_H')
    print()
    print('#include <stdint.h>')
    print()
    print(emit('g2d_qpu_copy_vec', blit))
    print()
    print(emit('g2d_qpu_alpha_vec', alpha))
    print()
    print(emit('g2d_qpu_present_vec', present))
    print()
    print('#endif')

    sys.stderr.write('  blit %d, alpha %d, present %d instrs\n' %
                     (len(blit), len(alpha), len(present)))


if __name__ == '__main__':
    main()
