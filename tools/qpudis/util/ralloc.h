/* Mesa ralloc 的最小替身：反汇编器只用到 ralloc_vasprintf_rewrite_tail()，
 * 它的语义是"在 str 的 offset 处追加格式化文本并更新 offset"。
 * 用 realloc + vsnprintf 直接实现，避免把整个 ralloc 移植过来。 */
#ifndef QPUDIS_RALLOC_STUB_H
#define QPUDIS_RALLOC_STUB_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline void ralloc_vasprintf_rewrite_tail(char** str, size_t* len,
                                                 const char* fmt, va_list ap) {
  va_list ap2;
  va_copy(ap2, ap);
  int n = vsnprintf(NULL, 0, fmt, ap2);
  va_end(ap2);
  if (n < 0) {
    return;
  }
  char* p = (char*)realloc(*str, *len + (size_t)n + 1);
  if (p == NULL) {
    return;
  }
  vsnprintf(p + *len, (size_t)n + 1, fmt, ap);
  *str = p;
  *len += (size_t)n;
}

static inline void* ralloc(void* ctx, size_t size) {
  (void)ctx;
  return malloc(size);
}

/* 反汇编器用它分配输出缓冲（初值需为 0） */
static inline void* rzalloc_size(void* ctx, size_t size) {
  (void)ctx;
  return calloc(1, size);
}

static inline void ralloc_free(void* p) {
  free(p);
}

#endif /* QPUDIS_RALLOC_STUB_H */
