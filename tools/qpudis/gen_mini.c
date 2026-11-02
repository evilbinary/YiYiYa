/* 生成两个最小内核：
 *   g2d_mini_read  —— 读源 (0,0) 再写出去（验证 TMU 读）
 *   g2d_mini_write —— 只把 uniform 里的常量写出去（验证内核框架 + 写路径）
 *
 * 对照意义：
 *   mini_write 通了、mini_read 不通  ⇒ 内核框架和写路径没问题，卡在 TMU 读
 *   两个都不通                      ⇒ dispatch 参数 / thrsw / uniform 延迟有问题
 *
 * uniform 约定：
 *   u[0] = Texture State 的 GPU VA       （mini_write 不用）
 *   u[1] = Sampler State 的 GPU VA       （mini_write 不用）
 *   u[2] = P2 控制字（Op=Regular 在 bits[23:20]）
 *   u[3] = 目标地址（写回的字节地址）
 *   u[4] = 读取坐标 (s | t<<16)          mini_write 里改成"要写的颜色值"
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
  if (!v3d_qpu_instr_pack(&g_dev, &in, &packed)) {
    fprintf(stderr, "pack failed (add waddr=%u rf=%u)\n", waddr, rfN);
    return 0;
  }
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
  if (!v3d_qpu_instr_pack(&g_dev, &in, &packed)) {
    fprintf(stderr, "pack failed (mul waddr=%u rf=%u)\n", waddr, rfN);
    return 0;
  }
  return packed;
}

/* nop + sig.wrtmuc：这是 Mesa 的 vir_WRTMUC 的实际形态 ——
 * 硬件把"下一个 uniform"的值写进 TMUC，并推进 descriptor 段号。
 * 注意：它不是 mov tmuc, rfN，那样写不会推进 descriptor 队列。 */
static uint64_t __attribute__((unused)) mk_wrtmuc(void) {
  struct v3d_qpu_instr in;
  memset(&in, 0, sizeof(in));
  in.type = V3D_QPU_INSTR_TYPE_ALU;
  in.alu.add.op = V3D_QPU_A_NOP;
  in.alu.add.b.raddr = 3;
  in.alu.mul.op = V3D_QPU_M_NOP;
  in.alu.mul.b.raddr = 63;
  in.sig.wrtmuc = true;
  uint64_t packed = 0;
  if (!v3d_qpu_instr_pack(&g_dev, &in, &packed)) {
    fprintf(stderr, "pack failed (wrtmuc)\n");
    return 0;
  }
  return packed;
}

#define K_NOP 0x38000000bb03f000ULL
#define K_THRSW 0x38200000bb03f000ULL
#define K_TMUWT 0x38001006bb03f00fULL
#define K_LDTMU_RF28 0x38870000bb03f000ULL

/* ldunifrf.rfN：目标寄存器在 word1 的 bits[17:14]，每 +1 寄存器 = +0x4000，
 * 换算到 64 位值的位移是 32+14 = 46。 */
static uint64_t ldunifrf(int idx) {
  return 0x39800000bb03f000ULL + ((uint64_t)idx << 46);
}

static void emit(const char* name, const uint64_t* prog, int n) {
  printf("/* %s: %d 条指令 */\n", name, n);
  printf("static const uint64_t %s[] = {\n", name);
  for (int i = 0; i < n; i++) {
    char buf[192];
    snprintf(buf, sizeof(buf), "%s", v3d_qpu_disasm(&g_dev, prog[i]));
    for (char* p = buf; *p; p++) {
      if (*p == '\n') {
        *p = ' ';
      }
    }
    printf("    0x%016llxULL, /* %2d: %s */\n", (unsigned long long)prog[i], i,
           buf);
  }
  printf("};\n");
  printf("static const unsigned %s_n = %d;\n\n", name, n);
}

int main(void) {
  memset(&g_dev, 0, sizeof(g_dev));
  g_dev.ver = 71;

  /* ---- 变体 A：只写 uniform 常量（验证框架 + 写路径） ---- */
  {
    uint64_t p[96];
    int n = 0;
    for (int i = 0; i <= 5; i++) {
      p[n++] = ldunifrf(i);
    }
    for (int i = 0; i < 6; i++) {
      p[n++] = K_NOP;
    }
    /* 【探针】先做一次完整的 TMU 读（结果丢弃）：
     * 如果接下来的写入仍然生效，说明读没有卡住线程、
     * 只是返回 0；如果写入不生效，说明读让线程停在了那里。
     * 这是唯一能把"读失败"和"线程卡死"分开的手段。 */
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUC, 0, 0); /* P0 = u0 */
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUC, 1, 0); /* P1 = u1 */
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUC, 2, 0); /* P2 = u2 */
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 4, 0); /* 坐标 = u4 */
    /* 【二分探针 · 阶段 1】只保留 3 条 mov tmuc + mov tmua，
     * 去掉 thrsw/ldtmu。如果接下来的写入生效，说明
     * descriptor 写入本身没问题，卡住的是 ldtmu；
     * 如果写入仍不生效，说明 mov tmuc 本身就会卡住线程。 */
    p[n++] = K_THRSW;
    p[n++] = K_LDTMU_RF28;
    p[n++] = K_NOP;
    p[n++] = K_NOP;
    /* 再写颜色（这一步已知能成功） */
    p[n++] = mk_mov_special_mul(V3D_QPU_WADDR_TMUD, 5); /* 数据 = u[5] */
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 3, 0); /* 目标 = u[3] */
    p[n++] = K_TMUWT;
    p[n++] = K_NOP;
    p[n++] = K_NOP;
    p[n++] = K_THRSW;
    p[n++] = K_NOP;
    p[n++] = K_NOP;
    p[n++] = K_NOP;
    emit("g2d_mini_write", p, n);
  }

  /* ---- 变体 B：读源 (0,0) 再写（验证 TMU 读） ---- */
  {
    uint64_t p[96];
    int n = 0;
    for (int i = 0; i <= 7; i++) {
      p[n++] = ldunifrf(i);
    }
    for (int i = 0; i < 6; i++) {
      p[n++] = K_NOP;
    }
    /* descriptor 三段：直接 mov 写 TMUC。
     * Mesa 用 nop.wrtmuc + 隐式 uniform，但那条路实测没有把值写进去（
     * P0/P1/P2 全是默认值）；这里改成 EwokOS 的写法，
     * 但配上我们修正过的 Texture State（swizzle 2/3/4/5、通道掩码 0xF）。 */
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUC, 5, 0); /* P0 = u5 */
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUC, 6, 0); /* P1 = u6 */
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUC, 7, 0); /* P2 = u7 */
    p[n++] = K_NOP;
    p[n++] = K_NOP;
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 4, 0); /* 坐标 */
    /* 【关键】THRSW 是"等待所有未完成 TMU 操作结束"的屏障。
     * Mesa 的 ntq_flush_tmu() 就是 thrsw -> ldtmu 这个顺序；
     * 缺了它，ldtmu 读到的是尚未就绪的寄存器（=0）。
     * EwokOS 那批内核只在结尾有一条 thrsw，读结果前没有。 */
    p[n++] = K_THRSW;
    /* 一次读 4 个分量：rf28=R, rf29=G, rf30=B, rf31=A */
    p[n++] = K_LDTMU_RF28;
    p[n++] = 0x38874000bb03f000ULL; /* ldtmu.rf29 */
    p[n++] = 0x38878000bb03f000ULL; /* ldtmu.rf30 */
    p[n++] = 0x3887c000bb03f000ULL; /* ldtmu.rf31 */
    p[n++] = K_NOP;
    p[n++] = K_NOP;
    p[n++] = K_NOP;
    /* 四个分量依次写到 dst[0..3]，一次就能看到全部分量 */
    p[n++] = mk_mov_special_mul(V3D_QPU_WADDR_TMUD, 28);
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 3, 0); /* dst+0  */
    p[n++] = mk_mov_special_mul(V3D_QPU_WADDR_TMUD, 29);
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 5, 0); /* dst+4  */
    p[n++] = mk_mov_special_mul(V3D_QPU_WADDR_TMUD, 30);
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 6, 0); /* dst+8  */
    p[n++] = mk_mov_special_mul(V3D_QPU_WADDR_TMUD, 31);
    p[n++] = mk_mov_special_add(V3D_QPU_WADDR_TMUA, 7, 0); /* dst+12 */
    p[n++] = K_TMUWT;
    p[n++] = K_NOP;
    p[n++] = K_NOP;
    p[n++] = K_THRSW;
    p[n++] = K_NOP;
    p[n++] = K_NOP;
    emit("g2d_mini_read", p, n);
  }

  return 0;
}
