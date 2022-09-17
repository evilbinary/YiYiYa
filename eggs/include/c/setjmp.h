#ifndef _SETJMP_H
#define _SETJMP_H 1

#ifdef ARM
typedef unsigned long long __jmp_buf[23];

typedef struct __jmp_buf_tag {
  __jmp_buf __jb;
  unsigned long __fl;
  unsigned long __ss[128 / sizeof(long)];
} jmp_buf[1];

#else
typedef struct {
  unsigned int regs[32];
} jmp_buf[1];

typedef struct {
  unsigned int regs[32];
} sigjmp_buf[1];

#endif

#ifdef __cplusplus
extern "C" {
#endif

void longjmp(jmp_buf, int);
// void   siglongjmp(sigjmp_buf, int);
// void  _longjmp(jmp_buf, int);

int setjmp(jmp_buf);
// int    sigsetjmp(sigjmp_buf, int);
// int   _setjmp(jmp_buf);

#ifdef __cplusplus
}
#endif

#endif
