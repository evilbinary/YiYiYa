#include "setjmp.h"

#ifdef X86
// int setjmp(jmp_buf env) {
//   asm volatile(
//       "mov %0, %%eax\n"
//       "movl %%ebp, (%%eax)\n"
//       "movl %%ebx, 0x4(%%eax)\n"
//       "movl %%edi, 0x8(%%eax)\n"
//       "movl %%esi, 0xC(%%eax)\n"

//       "movl %%ebp, %%ebx\n"
//       "add $0x4, %%ebx\n"
//       "movl (%%ebx), %%ecx\n"
//       "movl %%ecx, 0x10(%%eax)\n"
//       :
//       : "p"(&(env[0].regs))
//       : "eax", "ebx", "ecx");
//   return 0;
// }

// void longjmp(jmp_buf env, int val) {
//   asm volatile(
//       "mov %1, %%eax\n"
//       "mov %0, %%esp\n"

//       "mov (%%esp), %%ebp\n"
//       "mov 0x4(%%esp), %%ebx\n"
//       "mov 0x8(%%esp), %%edi\n"
//       "mov 0xC(%%esp), %%esi\n"
//       "mov 0x10(%%esp), %%ecx\n"

//       "leave\n"
//       "popl %%edx\n"
//       "pushl %%ecx\n"

//       "ret\n"

//       :
//       : "p"(&(env[0].regs)), "g"(val)
//       : "eax", "ecx", "esp");
// }
#elif defined(ARM)

  int __hwcap[38];

#else

#endif
