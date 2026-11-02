/* Mesa util/macros.h 的最小替身：QPU 反汇编只用到这三个宏。 */
#ifndef QPUDIS_MACROS_STUB_H
#define QPUDIS_MACROS_STUB_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define BITFIELD64_BIT(b) (UINT64_C(1) << (b))
/* bits 可能等于 64（ANYOPMASK = OP_RANGE(0,63)），此时不能移位 */
#define BITFIELD64_MASK(bits) \
  ((bits) >= 64 ? ~UINT64_C(0) : (BITFIELD64_BIT(bits) - 1))
/* Mesa 里第二个参数是【位数】不是结束位：
 *   OP_RANGE(bot, top)  = BITFIELD64_RANGE(bot, top - bot + 1) */
#define BITFIELD64_RANGE(start, bits) (BITFIELD64_MASK(bits) << (start))

#ifndef MIN2
#define MIN2(a, b) ((a) < (b) ? (a) : (b))
#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

#ifndef MAYBE_UNUSED
#define MAYBE_UNUSED __attribute__((unused))
#endif

#ifndef ATTRIBUTE_CONST
#define ATTRIBUTE_CONST __attribute__((const))
#endif
#ifndef ATTRIBUTE_PURE
#define ATTRIBUTE_PURE __attribute__((pure))
#endif
/* Mesa 的 ASSERTED 是"仅 debug 断言用"的前缀标记，展开为空即可 */
#ifndef ASSERTED
#define ASSERTED
#endif
#ifndef PRINTFLIKE
#define PRINTFLIKE(fmt, arg)
#endif

/* 反汇编器的 switch 兜底分支用它报"枚举值非法" */
#ifndef UNREACHABLE
#define UNREACHABLE(msg)           \
  do {                             \
    fprintf(stderr, "%s\n", msg);  \
    abort();                       \
  } while (0)
#endif

#ifndef unreachable
#define unreachable(msg) UNREACHABLE(msg)
#endif

/* Mesa 的 STATIC_ASSERT 是函数式编译期断言 */
#ifndef STATIC_ASSERT
#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)
#endif

#endif /* QPUDIS_MACROS_STUB_H */
