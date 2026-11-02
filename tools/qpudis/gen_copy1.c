/* 最小「直接地址读」验证内核（两个变体）。
 *
 * 依据是 argb_copy 的真实反汇编：
 *   [14] mov rf15, -4          ; 0x39c0000ff903f703
 *   [47] mov tmuc, rf15        ; TMUC = 0xFFFFFFFC   ← 注意：没有 wrtmuc 信号
 *   [48] mov tmua, rf14        ; TMUA = 源【字节地址】
 *   [59] ldtmu.rf20
 *
 * 变体 1（copy1）：完全照抄 argb_copy —— mov tmuc 不带 wrtmuc。
 * 变体 2（copy2）：在 mov tmuc 上加 sig.wrtmuc。
 *   Mesa 用 nop.wrtmuc 来提交 TMU 配置；不加这个信号，硬件可能不认这次
 *   写入，于是读操作沿用旧的无效配置并返回 0。argb_copy 恰好没加，
 *   而它正是 EwokOS 自述"仅在仿真器验证"的那批内核。
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "broadcom/common/v3d_device_info.h"
#include "broadcom/qpu/qpu_instr.h"
#include "broadcom/qpu/qpu_disasm.h"

static struct v3d_device_info g_dev;

static uint64_t mk_mov_special_add(uint8_t waddr, uint8_t rfN, int wrtmuc) {
  struct v3d_qpu_instr in;
  memset(&in, 0, sizeof(in));
  in.type = V3D_QPU_INSTR_TYPE_ALU;
  in.alu.add.op = V3D_QPU_A_MOV;
  in.alu.add.waddr = waddr;
  in.alu.add.magic_write = true;
  in.alu.add.a.raddr = rfN;
  in.alu.add.b.raddr = 3;
  in.alu.mul.op = V3D_QPU_M_NOP;
  in.alu.mul.b.raddr = 63;
  in.sig.wrtmuc = wrtmuc ? true : false;
  uint64_t packed = 0;
  v3d_qpu_instr_pack(&g_dev, &in, &packed);
  return packed;
}

static uint64_t mk_mov_special_mul(uint8_t waddr, uint8_t rfN) {
  struct v3d_qpu_instr in;
  memset(&in, 0, sizeof(in));
  in.type = V3D_QPU_INSTR_TYPE_ALU;
  in.alu.add.op = V3D_QPU_A_NOP;
  in.alu.add.b.raddr = 3;
  in.alu.mul.op = V3D_QPU_M_MOV;
  in.alu.mul.waddr = waddr;
  in.alu.mul.magic_write = true;
  in.alu.mul.a.raddr = rfN;
  in.alu.mul.b.raddr = 63;
  uint64_t packed = 0;
  v3d_qpu_instr_pack(&g_dev, &in, &packed);
  return packed;
}

#define K_NOP 0x38000000bb03f000ULL
#define K_THRSW 0x38200000bb03f000ULL
#define K_TMUWT 0x38001006bb03f00fULL
#define K_LDTMU_RF28 0x38870000bb03f000ULL
#define K_MOV_RF15_NEG4 0x39c0000ff903f703ULL /* argb_copy[14]: mov rf15, -4 */

static uint64_t ldunifrf(int idx) {
  return 0x39800000bb03f000ULL + ((uint64_t)idx << 46);
}

static int build(uint64_t* p, int with_wrtmuc) {
  int n = 0;
  p[n++] = ldunifrf(0); /* u0 = 源字节地址 */
  p[n++] = ldunifrf(1); /* u1 = 目标字节地址 */
  for (int i = 0; i < 4; i++) {
    p[n++] = K_NOP;
  }
  p[n++] = K_MOV_RF15_NEG4; /* rf15 = 0xFFFFFFFC */
  p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUC, 15, with_wrtmuc);
  p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 0, 0); /* tmua = src */
  p[n++] = K_LDTMU_RF28;
  for (int i = 0; i < 4; i++) {
    p[n++] = K_NOP;
  }
  p[n++] = mk_mov_special_mul(V3D_QPU_WADDR_TMUD, 28);
  p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 1, 0); /* tmua = dst */
  p[n++] = K_TMUWT;
  p[n++] = K_NOP;
  p[n++] = K_NOP;
  p[n++] = K_THRSW;
  p[n++] = K_NOP;
  p[n++] = K_NOP;
  return n;
}

static void emit(const char* name, const uint64_t* p, int n) {
  printf("static const uint64_t %s[] = {\n", name);
  for (int i = 0; i < n; i++) {
    char buf[192];
    snprintf(buf, sizeof(buf), "%s", v3d_qpu_disasm(&g_dev, p[i]));
    for (char* q = buf; *q; q++) {
      if (*q == '\n') {
        *q = ' ';
      }
    }
    printf("    0x%016llxULL, /* %2d: %s */\n", (unsigned long long)p[i], i, buf);
  }
  printf("};\n");
  printf("static const unsigned %s_n = %d;\n\n", name, n);
}

int main(void) {
  memset(&g_dev, 0, sizeof(g_dev));
  g_dev.ver = 71;

  uint64_t p[64];
  int n;

  printf("/* 直接地址读验证内核（自动生成） */\n\n");

  n = build(p, 0);
  emit("g2d_mini_copy1", p, n);

  n = build(p, 1);
  emit("g2d_mini_copy2", p, n);

  return 0;
}
