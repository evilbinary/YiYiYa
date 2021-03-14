#include <setjmp.h>

int setjmp(jmp_buf env) {
#ifdef __i386__
  asm volatile(
      "mov %0, %%eax\n"
      "movl %%ebp, (%%eax)\n"
      "movl %%ebx, 0x4(%%eax)\n"
      "movl %%edi, 0x8(%%eax)\n"
      "movl %%esi, 0xC(%%eax)\n"

      "movl %%ebp, %%ebx\n"
      "add $0x4, %%ebx\n"
      "movl (%%ebx), %%ecx\n"
      "movl %%ecx, 0x10(%%eax)\n"
      :
      : "p"(&(env[0].regs))
      : "eax", "ebx", "ecx");
#else
  os_freakout("setjmp not implemented for anything besides x86");
#endif

  return 0;
}

void longjmp(jmp_buf env, int val) {
#ifdef __i386__
  asm volatile(
      "mov %1, %%eax\n"
      "mov %0, %%esp\n"

      "mov (%%esp), %%ebp\n"
      "mov 0x4(%%esp), %%ebx\n"
      "mov 0x8(%%esp), %%edi\n"
      "mov 0xC(%%esp), %%esi\n"
      "mov 0x10(%%esp), %%ecx\n"

      "leave\n"
      "popl %%edx\n"
      "pushl %%ecx\n"

      "ret\n"

      :
      : "p"(&(env[0].regs)), "g"(val)
      : "eax", "ecx", "esp");
#else
  os_freakout("longjmp not implemented for anything besides x86");
#endif
}
