/* 坐标编码探测：读 4 个不同坐标，看哪个对应 src 的哪个像素。
 *
 * self-test 会先把 src[1] 写成 0x00000001、src[256] 写成 0x00000100，
 * 其余仍是 fill 的 0xff00ff00。于是：
 *   读 coords=0        → 0xff00ff00
 *   读 coords=1        → 若 s 在低 16 位 → 0x00000001
 *   读 coords=0x10000  → 若 t 在高 16 位 → 0x00000100
 *   读 coords=0x101    → 0x00000101（两者都对时的组合）
 *
 * 4 次读按请求顺序进输出 FIFO，4 条 ldtmu 按序取出。
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "broadcom/common/v3d_device_info.h"
#include "broadcom/qpu/qpu_instr.h"
#include "broadcom/qpu/qpu_disasm.h"

static struct v3d_device_info g_dev;

static uint64_t mk_mov_special_add(uint8_t waddr, uint8_t rfN) {
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

#define K_NOP 0x38000000bb03f000ULL
#define K_THRSW 0x38200000bb03f000ULL
#define K_TMUWT 0x38001006bb03f00fULL
#define K_LDTMU_RF28 0x38870000bb03f000ULL
#define K_LDTMU_RF29 0x38874000bb03f000ULL
#define K_LDTMU_RF30 0x38878000bb03f000ULL
#define K_LDTMU_RF31 0x3887c000bb03f000ULL

static uint64_t ldunifrf(int idx) {
  return 0x39800000bb03f000ULL + ((uint64_t)idx << 46);
}

int main(void) {
  memset(&g_dev, 0, sizeof(g_dev));
  g_dev.ver = 71;

  uint64_t p[128];
  int n = 0;

  for (int i = 0; i <= 10; i++) {
    p[n++] = ldunifrf(i);
  }
  for (int i = 0; i < 4; i++) {
    p[n++] = K_NOP;
  }

  /* 不配描述符：实证已证明 miniread1 里 wrtmuc 消费的 u5/u6/u7 全是 0
   * （CPU 没填），而读地址 u3=src_gva（裸字节地址）却读回了正确的
   * 0xff00ff00 —— TMU 默认就是裸字节地址模式。 */

  /* 按序发 4 个读请求（u4..u7 = 四个字节地址） */
  p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 4);
  p[n++] = K_NOP;
  p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 5);
  p[n++] = K_NOP;
  p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 6);
  p[n++] = K_NOP;
  p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 7);
  p[n++] = K_NOP;
  p[n++] = K_NOP;
  p[n++] = K_NOP;

  /* 按序取 4 个结果 */
  p[n++] = K_LDTMU_RF28;
  p[n++] = K_NOP;
  p[n++] = K_LDTMU_RF29;
  p[n++] = K_NOP;
  p[n++] = K_LDTMU_RF30;
  p[n++] = K_NOP;
  p[n++] = K_LDTMU_RF31;
  p[n++] = K_NOP;
  p[n++] = K_NOP;
  p[n++] = K_NOP;

  /* 依次写到 dst[0..3] */
  p[n++] = mk_mov_special_mul(V3D_QPU_WADDR_TMUD, 28);
  p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 0);
  p[n++] = mk_mov_special_mul(V3D_QPU_WADDR_TMUD, 29);
  p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 1);
  p[n++] = mk_mov_special_mul(V3D_QPU_WADDR_TMUD, 30);
  p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 2);
  p[n++] = mk_mov_special_mul(V3D_QPU_WADDR_TMUD, 31);
  p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 3);

  p[n++] = K_TMUWT;
  p[n++] = K_NOP;
  p[n++] = K_NOP;
  p[n++] = K_THRSW;
  p[n++] = K_NOP;
  p[n++] = K_NOP;

  printf("/* 坐标编码探测：%d 条指令 */\n", n);
  printf("static const uint64_t g2d_mini_write[] = {\n");
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
  printf("static const unsigned g2d_mini_write_n = %d;\n", n);
  return 0;
}
