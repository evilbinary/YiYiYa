#ifndef _SETJMP_H
#define _SETJMP_H			1


typedef struct
{
	unsigned int regs[32];
} jmp_buf[1];

typedef struct 
{
	unsigned int regs[32];
} sigjmp_buf[1];


#ifdef __cplusplus
extern "C" {
#endif

void   longjmp(jmp_buf, int);
void   siglongjmp(sigjmp_buf, int);
void  _longjmp(jmp_buf, int);


int    setjmp(jmp_buf);
int    sigsetjmp(sigjmp_buf, int);
int   _setjmp(jmp_buf);


#ifdef __cplusplus
}
#endif


#endif

