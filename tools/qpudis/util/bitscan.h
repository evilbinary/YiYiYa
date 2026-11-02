/* Mesa util/bitscan.h 的最小替身。qpu_pack.c 只是 include 了它，
 * 实际没有调用（反汇编路径用不到位扫描）。 */
#ifndef QPUDIS_BITSCAN_STUB_H
#define QPUDIS_BITSCAN_STUB_H

#include <stdint.h>

static inline int util_last_bit(uint32_t v) {
  return v ? (32 - __builtin_clz(v)) : 0;
}

static inline int util_logbase2(uint32_t v) {
  return v ? (31 - __builtin_clz(v)) : 0;
}

static inline uint32_t util_next_power_of_two(uint32_t v) {
  if (v <= 1) {
    return 1;
  }
  return 1u << (32 - __builtin_clz(v - 1));
}

#endif /* QPUDIS_BITSCAN_STUB_H */
