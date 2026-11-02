/* 最小 QPU 指令构造器 + 自校验。
 *
 * 目的：在往内核里插入/替换指令之前，先证明"从字段构造出的机器码"与
 * 反汇编器认得的完全一致。全部自检通过后，才用它生成真正的内核。
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "broadcom/common/v3d_device_info.h"
#include "broadcom/qpu/qpu_instr.h"
#include "broadcom/qpu/qpu_disasm.h"

static struct v3d_device_info g_dev;

static void alu_init(struct v3d_qpu_instr* in) {
  memset(in, 0, sizeof(*in));
  in->type = V3D_QPU_INSTR_TYPE_ALU;
  /* 双发射的另一半填 NOP，raddr_b 默认 63 —— 与现成内核里的空槽一致 */
  in->alu.add.op = V3D_QPU_A_NOP;
  in->alu.mul.op = V3D_QPU_M_NOP;
  in->alu.add.b.raddr = 3;
  in->alu.mul.b.raddr = 63;
}

/* mov <magic waddr>, rfN  —— 写 TMU 特殊寄存器（add 槽） */
static uint64_t mk_mov_special_add(uint8_t waddr, uint8_t rfN) {
  struct v3d_qpu_instr in;
  alu_init(&in);
  in.alu.add.op = V3D_QPU_A_MOV;
  in.alu.add.waddr = waddr;
  in.alu.add.magic_write = true;
  in.alu.add.a.raddr = rfN;
  uint64_t packed = 0;
  if (!v3d_qpu_instr_pack(&g_dev, &in, &packed)) {
    return 0;
  }
  return packed;
}

/* mov <magic waddr>, rfN —— mul 槽版本（TMUD 走这里） */
static uint64_t mk_mov_special_mul(uint8_t waddr, uint8_t rfN) {
  struct v3d_qpu_instr in;
  alu_init(&in);
  in.alu.mul.op = V3D_QPU_M_MOV;
  in.alu.mul.waddr = waddr;
  in.alu.mul.magic_write = true;
  in.alu.mul.a.raddr = rfN;
  uint64_t packed = 0;
  if (!v3d_qpu_instr_pack(&g_dev, &in, &packed)) {
    return 0;
  }
  return packed;
}

int main(void) {
  memset(&g_dev, 0, sizeof(g_dev));
  g_dev.ver = 71;

  /* 校验方式：反汇编出来的文本里必须出现期望的指令助记符。
   * 不能直接比 64 位值 —— 因为真实内核大量使用双发射（add+mul），
   * 我们只构造了其中一半，add 槽是 nop。 */
  struct {
    const char* want_text;
    uint64_t got;
  } tests[] = {
      {"mov tmua, rf26", mk_mov_special_add(V3D_QPU_WADDR_TMUA, 26)},
      {"mov tmud, rf0", mk_mov_special_mul(V3D_QPU_WADDR_TMUD, 0)},
      {"mov tmuc, rf0", mk_mov_special_add(V3D_QPU_WADDR_TMUC, 0)},
      {"mov tmuc, rf31", mk_mov_special_add(V3D_QPU_WADDR_TMUC, 31)},
  };

  int bad = 0;
  for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%s", v3d_qpu_disasm(&g_dev, tests[i].got));
    int ok = (strstr(buf, tests[i].want_text) != NULL);
    printf("%-16s -> 0x%016llx  [%s] %s\n", tests[i].want_text,
           (unsigned long long)tests[i].got, buf, ok ? "OK" : "MISMATCH");
    if (!ok) {
      bad++;
    }
  }

  /* TMUC（waddr=32）没有现成样本可比，只打印出来供人工核对 */
  printf("\nmov tmuc, rf0  = 0x%016llx  (%s)\n",
         (unsigned long long)mk_mov_special_add(V3D_QPU_WADDR_TMUC, 0),
         v3d_qpu_disasm(&g_dev, mk_mov_special_add(V3D_QPU_WADDR_TMUC, 0)));

  printf("\n%s\n", bad ? "SELF-CHECK FAILED" : "SELF-CHECK PASSED");
  return bad ? 1 : 0;
}
