/* host 端 QPU 反汇编：把 v3d_qpu_kernels.h 里的 blob 逐条打印成汇编，
 * 用来确认某个内核到底从哪些 uniform 槽位取参数。
 *
 * 用法：
 *   ./qpudis argb_blit [起 止]      # 只打印区间
 *   ./qpudis argb_blit              # 全部
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "broadcom/common/v3d_device_info.h"
#include "broadcom/qpu/qpu_instr.h"
#include "broadcom/qpu/qpu_disasm.h"

/* 直接 include 内核 blob（只有数据定义） */
#include "../../foot/driver/gpu/v3d/v3d_qpu_kernels.h"
#include "extra_kernels.h"

static const uint64_t* pick(const char* name, unsigned* n) {
  if (strcmp(name, "argb_fill") == 0) {
    *n = g2d_qpu_argb_fill_n;
    return g2d_qpu_argb_fill;
  }
  if (strcmp(name, "argb_blit") == 0) {
    *n = g2d_qpu_argb_blit_n;
    return g2d_qpu_argb_blit;
  }
  if (strcmp(name, "argb_alpha") == 0) {
    *n = g2d_qpu_argb_alpha_n;
    return g2d_qpu_argb_alpha;
  }
  if (strcmp(name, "argb_copy") == 0) {
    *n = g2d_qpu_argb_copy_n;
    return g2d_qpu_argb_copy;
  }
  /* 从 EwokOS 提取的参照内核（gauss 是"读源+计算+写回"的标准样板） */
#define PICK_EXTRA(s)                     \
  if (strcmp(name, #s) == 0) {            \
    *n = k_##s##_n;                       \
    return k_##s;                         \
  }
  PICK_EXTRA(gauss_v7)
  PICK_EXTRA(gauss_h5)
  PICK_EXTRA(argb_scale_pow2)
  PICK_EXTRA(argb_rotate)
  PICK_EXTRA(argb_fill4)
  PICK_EXTRA(argb_rot90)
#undef PICK_EXTRA
  return NULL;
}

/* 往返测试：unpack(64位) -> pack(64位)，逐条比较。
 * 只有全部还原，才说明我们用 pack 构造新指令是可靠的 —— 这是后面
 * "插入 mov tmuc 指令" 和 "写自己的内核" 的前提。 */
static int roundtrip(const char* name) {
  unsigned n = 0;
  const uint64_t* k = pick(name, &n);
  if (k == NULL) {
    fprintf(stderr, "unknown kernel: %s\n", name);
    return 2;
  }
  struct v3d_device_info devinfo;
  memset(&devinfo, 0, sizeof(devinfo));
  devinfo.ver = 71;

  unsigned bad = 0;
  for (unsigned i = 0; i < n; i++) {
    struct v3d_qpu_instr instr;
    memset(&instr, 0, sizeof(instr));
    if (!v3d_qpu_instr_unpack(&devinfo, k[i], &instr)) {
      printf("[%3u] 0x%016llx  UNPACK FAILED\n", i, (unsigned long long)k[i]);
      bad++;
      continue;
    }
    uint64_t repacked = 0;
    if (!v3d_qpu_instr_pack(&devinfo, &instr, &repacked)) {
      printf("[%3u] 0x%016llx  PACK FAILED\n", i, (unsigned long long)k[i]);
      bad++;
      continue;
    }
    if (repacked != k[i]) {
      printf("[%3u] 0x%016llx -> 0x%016llx  MISMATCH\n", i,
             (unsigned long long)k[i], (unsigned long long)repacked);
      bad++;
    }
  }
  printf("%s: %u/%u ok, %u bad\n", name, n - bad, n, bad);
  return bad ? 1 : 0;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <kernel> [start] [end]\n"
                    "       %s --roundtrip <kernel|all>\n", argv[0], argv[0]);
    return 2;
  }

  /* --fields <kernel> <idx>：打印单条指令解出的全部字段，
   * 用来精确掌握编码规律（尤其是 magic write 的 waddr/raddr）。 */
  if (strcmp(argv[1], "--fields") == 0 && argc >= 4) {
    unsigned n = 0;
    const uint64_t* k = pick(argv[2], &n);
    if (k == NULL) {
      fprintf(stderr, "unknown kernel: %s\n", argv[2]);
      return 2;
    }
    unsigned idx = (unsigned)atoi(argv[3]);
    if (idx >= n) {
      return 2;
    }
    struct v3d_device_info devinfo;
    memset(&devinfo, 0, sizeof(devinfo));
    devinfo.ver = 71;
    struct v3d_qpu_instr in;
    memset(&in, 0, sizeof(in));
    if (!v3d_qpu_instr_unpack(&devinfo, k[idx], &in)) {
      printf("unpack failed\n");
      return 1;
    }
    printf("[%u] 0x%016llx  %s\n", idx, (unsigned long long)k[idx],
           v3d_qpu_disasm(&devinfo, k[idx]));
    printf("  type=%d\n", (int)in.type);
    if (in.type == V3D_QPU_INSTR_TYPE_ALU) {
      printf("  add: op=%-28s waddr=%u magic=%d a:raddr=%u unpack=%d "
             "b:raddr=%u\n",
             v3d_qpu_add_op_name(in.alu.add.op), in.alu.add.waddr,
             (int)in.alu.add.magic_write, in.alu.add.a.raddr, (int)in.alu.add.a.unpack,
             in.alu.add.b.raddr);
      printf("  mul: op=%-28s waddr=%u magic=%d a:raddr=%u unpack=%d "
             "b:raddr=%u\n",
             v3d_qpu_mul_op_name(in.alu.mul.op), in.alu.mul.waddr,
             (int)in.alu.mul.magic_write, in.alu.mul.a.raddr, (int)in.alu.mul.a.unpack,
             in.alu.mul.b.raddr);
      printf("  sig: wrtmuc=%d ldtmu=%d thrsw=%d\n", (int)in.sig.wrtmuc,
             (int)in.sig.ldtmu, (int)in.sig.thrsw);
    }
    return 0;
  }

  if (strcmp(argv[1], "--roundtrip") == 0) {
    if (argc < 3) {
      return 2;
    }
    if (strcmp(argv[2], "all") == 0) {
      static const char* const all[] = {"argb_fill", "argb_blit", "argb_alpha",
                                        "argb_copy", "gauss_v7", "gauss_h5"};
      int rc = 0;
      for (unsigned i = 0; i < sizeof(all) / sizeof(all[0]); i++) {
        rc |= roundtrip(all[i]);
      }
      return rc;
    }
    return roundtrip(argv[2]);
  }
  unsigned n = 0;
  const uint64_t* k = pick(argv[1], &n);
  if (k == NULL) {
    fprintf(stderr, "unknown kernel: %s\n", argv[1]);
    return 2;
  }
  unsigned lo = (argc > 2) ? (unsigned)atoi(argv[2]) : 0;
  unsigned hi = (argc > 3) ? (unsigned)atoi(argv[3]) : n;
  if (hi > n) {
    hi = n;
  }

  /* Pi5 = V3D 7.1 */
  struct v3d_device_info devinfo;
  memset(&devinfo, 0, sizeof(devinfo));
  devinfo.ver = 71;

  printf("=== %s: %u instructions (showing %u..%u) ===\n", argv[1], n, lo, hi);
  for (unsigned i = lo; i < hi; i++) {
    printf("[%3u] 0x%016llx  %s\n", i, (unsigned long long)k[i],
           v3d_qpu_disasm(&devinfo, k[i]));
  }
  return 0;
}
