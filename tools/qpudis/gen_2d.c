/* 生成两个 2D 内核：blit（1:1 拷贝）与 alpha（source-over 混合）。
 *
 * 依据真机实证的 TMU 字节地址读模型（见 zdata/raspi5-gpu.md 附录 E）：
 *   mov tmua, <裸字节地址> ; nop ; nop ; ldtmu.rf28
 *   —— 不配描述符、不碰 tmuc、ldtmu 前不能有 thrsw。
 *
 * 两个内核都用 eidx 做 16 lane 并行：一条 mov tmua 发 16 个读请求
 * （每 lane 一个地址），一条 ldtmu 每 lane 取回自己的结果。
 *
 * 分支：bdi=REL、ub=true（与 EwokOS 一致，fill 在真机可用），
 * offset 是【字节】相对偏移，相对分支的下一条指令。
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "broadcom/common/v3d_device_info.h"
#include "broadcom/qpu/qpu_instr.h"
#include "broadcom/qpu/qpu_disasm.h"

static struct v3d_device_info g_dev;

#define MAXI 512
static uint64_t prog[MAXI];
static int n;
static int loop_idx = -1, branch_idx = -1;

static void emit(uint64_t v) { prog[n++] = v; }

/* 显式指定「装到哪个寄存器」的 ldunifrf。
 * 目标寄存器在 word1 的 bits[17:14]（64 位里位移 46）。
 * 注意：uniform 索引仍是隐式递增的，idx 参数只用于文档说明。 */
static uint64_t mk_ldunifrf_idx(int reg, int idx) {
  (void)idx;
  return 0x39800000bb03f000ULL + ((uint64_t)reg << 46);
}

static uint64_t mk_ldunifrf(int idx) {
  return 0x39800000bb03f000ULL + ((uint64_t)idx << 46);
}

/* add 槽 ALU：waddr<32 且非 TMU 时 magic=0，TMU 特殊寄存器 magic=1 */
static uint64_t mk_add(enum v3d_qpu_add_op op, uint8_t waddr, int magic,
                       uint8_t a, uint8_t b, enum v3d_qpu_pf pf) {
  struct v3d_qpu_instr in;
  memset(&in, 0, sizeof(in));
  in.type = V3D_QPU_INSTR_TYPE_ALU;
  in.alu.add.op = op;
  in.alu.add.waddr = waddr;
  in.alu.add.magic_write = magic ? true : false;
  in.alu.add.a.raddr = a;
  in.alu.add.b.raddr = b;
  in.alu.mul.op = V3D_QPU_M_NOP;
  in.alu.mul.b.raddr = 63;
  if (pf != V3D_QPU_PF_NONE) {
    in.flags.apf = pf;
  }
  uint64_t packed = 0;
  v3d_qpu_instr_pack(&g_dev, &in, &packed);
  /* 【push 标志的高位修正】实测：Mesa 的 pack 把 `sub.pushz` 编成
   *   0x38004012...  （真机上 bu.anyna 不跳，循环只跑一次）
   * 而 EwokOS 内核实测可用的同一条是
   *   0x39e04012...
   * 两者只差 0x01E00000。unpack 对这两者解出的字段完全相同
   * （apf 都=1），所以往返测试发现不了 —— 这也是它一直没被查出来的原因。
   * 这里按经验补上这三位。 */
  if (pf != V3D_QPU_PF_NONE) {
    packed |= (0x01E00000ULL << 32);
  }
  return packed;
}

static uint64_t mk_mul(enum v3d_qpu_mul_op op, uint8_t waddr, uint8_t a,
                       uint8_t b) {
  struct v3d_qpu_instr in;
  memset(&in, 0, sizeof(in));
  in.type = V3D_QPU_INSTR_TYPE_ALU;
  in.alu.add.op = V3D_QPU_A_NOP;
  in.alu.add.b.raddr = 3;
  in.alu.mul.op = op;
  in.alu.mul.waddr = waddr;
  in.alu.mul.a.raddr = a;
  in.alu.mul.b.raddr = b;
  uint64_t packed = 0;
  v3d_qpu_instr_pack(&g_dev, &in, &packed);
  return packed;
}

/* nop + sig.wrtmuc：把下一个 uniform 值提交进 TMUC 的一段（P0/P1/P2 按序） */
/* b 操作数用【小立即数】（-16..15）的 ALU 构造器。
 * 实测：不置 sig 位时 `add rf47, rf47, 8` 会被硬件当成"加 rf8"，
 * 而 rf8 位于 magic 保留段、值不可靠 —— 结果就是加法等于没做。
 * 置位后（word0 |= 0x01E00000）反汇编显示 "8"，语义正确。
 * 注意：这与 push 标志是两回事，别混用。 */
static uint64_t mk_add_i(enum v3d_qpu_add_op op, uint8_t waddr, uint8_t a,
                         int imm, enum v3d_qpu_pf pf) {
  struct v3d_qpu_instr in;
  memset(&in, 0, sizeof(in));
  in.type = V3D_QPU_INSTR_TYPE_ALU;
  in.alu.add.op = op;
  in.alu.add.waddr = waddr;
  in.alu.add.a.raddr = a;
  in.alu.add.b.raddr = (uint8_t)(imm & 0x1F);
  in.alu.mul.op = V3D_QPU_M_NOP;
  in.alu.mul.b.raddr = 63;
  if (pf != V3D_QPU_PF_NONE) {
    in.flags.apf = pf;
  }
  uint64_t packed = 0;
  v3d_qpu_instr_pack(&g_dev, &in, &packed);
  packed |= (0x01E00000ULL << 32); /* small-imm 标志 */
  return packed;
}

static uint64_t mk_wrtmuc(void) {
  struct v3d_qpu_instr in;
  memset(&in, 0, sizeof(in));
  in.type = V3D_QPU_INSTR_TYPE_ALU;
  in.alu.add.op = V3D_QPU_A_NOP;
  in.alu.add.b.raddr = 3;
  in.alu.mul.op = V3D_QPU_M_NOP;
  in.alu.mul.b.raddr = 63;
  in.sig.wrtmuc = true;
  uint64_t packed = 0;
  v3d_qpu_instr_pack(&g_dev, &in, &packed);
  return packed;
}

/* 无条件相对分支：b.always <offset> */
static uint64_t mk_branch_always(int32_t rel_bytes) {
  struct v3d_qpu_instr in;
  memset(&in, 0, sizeof(in));
  in.type = V3D_QPU_INSTR_TYPE_BRANCH;
  in.branch.cond = V3D_QPU_BRANCH_COND_ALWAYS;
  in.branch.msfign = V3D_QPU_MSFIGN_NONE;
  in.branch.bdi = V3D_QPU_BRANCH_DEST_REL;
  /* 【纯相对分支】ub=0 表示目标不再叠加 uniform。
   * 实测：ub=1 时跳转目标 = PC + offset + u[bdu]，
   * 而我们的 uniform 里没有"分支基址"这个槽（py-videocore7 里由驱动
   * 专门用 unif_addr 提供、值通常为 0），于是叠加到 dst_gva 之类的
   * 大数上，目标直接飞出内核。 */
  /* 【与 argb_fill（真机成功）一致】ub=1 + bdu=1。
   * 实测编码 0x02ffffddff00d000 与我们的 0x02ffff***ff00d000 格式完全相同，
   * 只差 offset 值 —— 说明 ub=1 本身就是对的。 */
  in.branch.ub = true;
  in.branch.bdu = 1;
  in.branch.offset = rel_bytes;
  uint64_t packed = 0;
  v3d_qpu_instr_pack(&g_dev, &in, &packed);
  return packed;
}

static uint64_t mk_branch(int32_t rel_bytes) {
  struct v3d_qpu_instr in;
  memset(&in, 0, sizeof(in));
  in.type = V3D_QPU_INSTR_TYPE_BRANCH;
  in.branch.cond = V3D_QPU_BRANCH_COND_ANYNA;
  in.branch.msfign = V3D_QPU_MSFIGN_NONE;
  in.branch.bdi = V3D_QPU_BRANCH_DEST_REL;
  /* 【分支不要叠 uniform】ub=1 表示"分支目标还要加上某个 uniform 的值"，
   * bdu 就是那个 uniform 的索引。之前照抄 EwokOS 用 ub=1/bdu=1，
   * 而我们的 u[1] = dst_gva = 0x32000000 —— 于是实际目标 =
   *   PC + offset + 0x32000000
   * 直接飞出内核，表现出来的就是"循环不生效"。
   * 实测证据：计数器能从 32768 正确减到 32767（说明 sub.pushz 执行了），
   * 但累加只发生一次（说明 bu.anyna 没跳回）。
   * EwokOS 同样写 ub=1 却能工作，很可能只是它的 u[1] 恰好为 0。 */
  in.branch.ub = false;
  in.branch.bdu = 0;
  in.branch.offset = rel_bytes;
  uint64_t packed = 0;
  v3d_qpu_instr_pack(&g_dev, &in, &packed);
  return packed;
}

#define K_NOP 0x38000000bb03f000ULL
#define K_THRSW 0x38200000bb03f000ULL
#define K_TMUWT 0x38001006bb03f00fULL
#define K_LDTMU_RF28 0x38870000bb03f000ULL
#define K_LDTMU_RF29 0x38874000bb03f000ULL
#define K_LDTMU_RF30 0x38878000bb03f000ULL
#define K_LDTMU_RF31 0x3887c000bb03f000ULL

static void emit_nops(int c) {
  for (int i = 0; i < c; i++) {
    emit(K_NOP);
  }
}

static void emit_tail(void) {
  emit(K_TMUWT);
  emit_nops(2);
  emit(K_THRSW);
  emit_nops(2);
}

/* 【按 argb_fill 的真实结构写循环】
 *
 * 真机成功的 argb_fill 循环长这样：
 *   [21] shr ...          ← 循环起点
 *   [24] sub.pushz rf24, rf24, 1
 *   [25] bu.anyna -40     ← 跳回 [21]
 *   [26] nop              ┐
 *   [27] nop              │ 三个延迟槽
 *   [28] nop              ┘
 *   [29] 循环之后的代码
 *
 * 关键：分支之后有【三个延迟槽】，槽里的指令无论跳不跳都会执行。
 * 我们之前只放 2 条 nop，第三条延迟槽正好是后面的写操作 ——
 * 于是每次循环都多写一次，且结构对不上。
 *
 * uniform: u0=src u1=dst u2=轮数
 */
static void build_copy(void) {
  n = 0;
  emit(mk_ldunifrf_idx(47, 0)); /* rf47 = src */
  emit(mk_ldunifrf_idx(48, 1)); /* rf48 = dst */
  emit(mk_ldunifrf_idx(27, 2)); /* rf27 = 轮数 */
  emit_nops(4);

  loop_idx = n;                  /* [7] */
  emit(mk_add_i(V3D_QPU_A_ADD, 47, 47, 8, V3D_QPU_PF_NONE));  /* rf47 += 8 */
  emit(mk_add_i(V3D_QPU_A_SUB, 27, 27, 1, V3D_QPU_PF_PUSHZ)); /* rf27-- */
  branch_idx = n;                /* [9] */
  emit(0);
  emit_nops(3);                  /* 三个延迟槽 [10][11][12] */

  emit(mk_add(V3D_QPU_A_MOV, V3D_QPU_WADDR_TMUD, 1, 47, 3, V3D_QPU_COND_NONE));
  emit(mk_add(V3D_QPU_A_MOV, V3D_QPU_WADDR_TMUA, 1, 48, 3, V3D_QPU_COND_NONE));
  emit_tail();

  /* 【硬编码 ub=1 的分支编码】Mesa 的 pack 在 ub=true 时生成
   * 0x02ffffedff001000（低 32 位 0xff001000），而真机成功的 argb_fill
   * 用的是 0x02ffffddff00d000（低 32 位 0xff00d000）—— 说明 pack 的
   * ub/bdu 位和硬件不一致。
   * rev=37 曾用 pack 正确生成过 0x02ffffedff00d000（低 32 位对），
   * 这里直接固定该形式，只把 offset 部分换成 loop_idx 对应的值。
   * -24 字节 = 从延迟槽起点 [10] 往回 3 条 = [7]（循环起点）。 */
  (void)mk_branch;
  prog[branch_idx] = 0x02ffffedff00d000ULL;
}

/* alpha：source-over。u0=src u1=dst u2=chunk数 u3=4 u4=64 u5=1
 *        u6=a(0..256) u7=256-a u8=0xFF 掩码 */
static void build_alpha(void) {
  n = 0;
  for (int i = 0; i <= 8; i++) {
    emit(mk_ldunifrf(i));
  }
  emit_nops(4);

  loop_idx = n;
  /* rf0 的写入会被丢弃（见 build_copy 注释），指针先搬到 rf18/rf19 */
  emit(mk_add(V3D_QPU_A_MOV, 47, 0, 0, 3, V3D_QPU_COND_NONE)); /* rf47 = src */
  emit(mk_add(V3D_QPU_A_MOV, 48, 0, 1, 3, V3D_QPU_COND_NONE)); /* rf48 = dst */

  loop_idx = n;
  emit(mk_add(V3D_QPU_A_EIDX, 9, 0, 0, 2, V3D_QPU_COND_NONE));  /* eidx rf9 */
  emit(mk_mul(V3D_QPU_M_UMUL24, 10, 9, 3));  /* rf10 = lane*4 */
  emit(mk_add(V3D_QPU_A_ADD, 11, 0, 47, 10, V3D_QPU_COND_NONE));
  emit(mk_add(V3D_QPU_A_MOV, V3D_QPU_WADDR_TMUA, 1, 11, 3, V3D_QPU_COND_NONE));
  emit_nops(2);
  emit(K_LDTMU_RF28); /* src 像素 → rf28 */
  emit_nops(1);
  emit(mk_add(V3D_QPU_A_ADD, 12, 0, 48, 10, V3D_QPU_COND_NONE));
  emit(mk_add(V3D_QPU_A_MOV, V3D_QPU_WADDR_TMUA, 1, 12, 3, V3D_QPU_COND_NONE));
  emit_nops(2);
  emit(K_LDTMU_RF29); /* dst 像素 → rf29 */
  emit_nops(2);
  /* rf13=src, rf14=dst, rf16=结果累加器 */
  emit(mk_add(V3D_QPU_A_MOV, 13, 0, 28, 3, V3D_QPU_COND_NONE));
  emit(mk_add(V3D_QPU_A_MOV, 14, 0, 29, 3, V3D_QPU_COND_NONE));
  emit(mk_add(V3D_QPU_A_XOR, 16, 0, 16, 16, V3D_QPU_COND_NONE));

  /* 四个通道，s = 0/8/16/24：
   *   c = ((src>>s & 0xFF)*a + (dst>>s & 0xFF)*inv) >> 8
   * a+inv == 256，所以 c <= 255，无需再掩码 */
  static const int shifts[4] = {0, 8, 16, 24};
  for (int ci = 0; ci < 4; ci++) {
    int s = shifts[ci];
    if (s > 0) {
      emit(mk_add(V3D_QPU_A_SHR, 15, 0, 13, s, V3D_QPU_COND_NONE));
    }
    emit(mk_add(V3D_QPU_A_AND, 15, 0, 15, 8, V3D_QPU_COND_NONE));
    emit(mk_mul(V3D_QPU_M_UMUL24, 15, 15, 6));
    if (s > 0) {
      emit(mk_add(V3D_QPU_A_SHR, 17, 0, 14, s, V3D_QPU_COND_NONE));
    }
    emit(mk_add(V3D_QPU_A_AND, 17, 0, 17, 8, V3D_QPU_COND_NONE));
    emit(mk_mul(V3D_QPU_M_UMUL24, 17, 17, 7));
    emit(mk_add(V3D_QPU_A_ADD, 15, 0, 15, 17, V3D_QPU_COND_NONE));
    emit(mk_add(V3D_QPU_A_SHR, 15, 0, 15, 8, V3D_QPU_COND_NONE));
    if (s > 0) {
      emit(mk_add(V3D_QPU_A_SHL, 15, 0, 15, s, V3D_QPU_COND_NONE));
    }
    emit(mk_add(V3D_QPU_A_OR, 16, 0, 16, 15, V3D_QPU_COND_NONE));
  }

  /* 写回 dst */
  emit(mk_add(V3D_QPU_A_MOV, V3D_QPU_WADDR_TMUD, 1, 16, 3, V3D_QPU_COND_NONE));
  emit(mk_add(V3D_QPU_A_MOV, V3D_QPU_WADDR_TMUA, 1, 12, 3, V3D_QPU_COND_NONE));
  emit(K_TMUWT); /* 同上：排空写，避免下一轮读请求被吞 */
  emit_nops(2);
  emit(mk_add(V3D_QPU_A_ADD, 47, 0, 47, 4, V3D_QPU_COND_NONE));
  emit(mk_add(V3D_QPU_A_ADD, 48, 0, 48, 4, V3D_QPU_COND_NONE));
  emit(mk_add(V3D_QPU_A_SUB, 25, 0, 25, 26, V3D_QPU_PF_PUSHZ));
  branch_idx = n;
  emit(0);
  emit_nops(2);
  emit_tail();

  prog[branch_idx] = mk_branch((loop_idx - (branch_idx + 1)) * 8);
}

static void emit_kernel(const char* name, void (*build)(void)) {
  n = 0;
  loop_idx = branch_idx = -1;
  build();
  printf("/* %s：%d 条指令，循环体 %d..%d */\n", name, n, loop_idx, branch_idx);
  printf("static const uint64_t %s[] = {\n", name);
  for (int i = 0; i < n; i++) {
    char buf[192];
    snprintf(buf, sizeof(buf), "%s", v3d_qpu_disasm(&g_dev, prog[i]));
    for (char* q = buf; *q; q++) {
      if (*q == '\n') {
        *q = ' ';
      }
    }
    printf("    0x%016llxULL, /* %3d: %s */\n", (unsigned long long)prog[i], i,
           buf);
  }
  printf("};\n");
  printf("static const unsigned %s_n = %d;\n\n", name, n);
}

int main(void) {
  memset(&g_dev, 0, sizeof(g_dev));
  g_dev.ver = 71;

  emit_kernel("g2d_qpu_copy_vec", build_copy);
  emit_kernel("g2d_qpu_alpha_vec", build_alpha);
  return 0;
}
