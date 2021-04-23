/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "exceptions.h"

#include "thread.h"

interrupt_handler_t *exception_handlers[IDT_NUMBER];
void exception_regist(u32 vec, interrupt_handler_t handler) {
  exception_handlers[vec] = handler;
}

void exception_info(interrupt_context_t *context) {
#ifdef ARM

#elif defined(X86)
  static const char *exception_msg[] = {
      "DIVIDE ERROR",      "DEBUG EXCEPTION",
      "BREAKPOINT",        "NMI",
      "OVERFLOW",          "BOUNDS CHECK",
      "INVALID OPCODE",    "COPROCESSOR NOT VALID",
      "DOUBLE FAULT",      "OVERRUN",
      "INVALID TSS",       "SEGMENTATION NOT PRESENT",
      "STACK EXCEPTION",   "GENERAL PROTECTION",
      "PAGE FAULT",        "REVERSED",
      "COPROCESSOR_ERROR",
  };
  cls();
  u32 cr2, cr3;
  u32 ds, es, fs, gs;
  asm volatile("movl	%%cr2,	%%eax" : "=a"(cr2));
  asm volatile("movl %%cr3,	%%eax" : "=a"(cr3));
  asm volatile("movl %%ds,	%%eax" : "=a"(ds));
  asm volatile("movl %%es,	%%eax" : "=a"(es));
  asm volatile("movl %%fs,	%%eax" : "=a"(fs));
  asm volatile("movl %%gs,	%%eax" : "=a"(gs));
  if (context->no != 14) {
    thread_t *current = thread_current();
    if (context->no < sizeof exception_msg) {
      kprintf("EXCEPTION %d: %s\n=======================\n", context->no,
              exception_msg[context->no]);
    } else {
      kprintf("INTERRUPT %d\n=======================\n", context->no);
    }
    if (current != NULL) {
      kprintf("tid:%d\n", current->id);
    }
    kprintf("cs:\t%x\teip:\t%x\teflags:\t%x\n", context->cs, context->eip,
            context->eflags);
    kprintf("ss:\t%x\tesp:\t%x\n", context->ss, context->esp);
    // kprintf("old ss:\t%x\told esp:%x\n", old_ss, old_esp);
    kprintf("code:%x\tcr2:\t%x\tcr3:\t%x\n", context->no, cr2, cr3);
    kprintf("General Registers:\n=======================\n");
    kprintf("eax:\t%x\tebx:\t%x\n", context->eax, context->ebx);
    kprintf("ecx:\t%x\tedx:\t%x\n", context->ecx, context->edx);
    kprintf("esi:\t%x\tedi:\t%x\tebp:\t%x\n", context->esi, context->edi,
            context->ebp);
    kprintf("Segment Registers:\n=======================\n");
    kprintf("ds:\t%x\tes:\t%x\n", ds, es);
    kprintf("fs:\t%x\tgs:\t%x\n", fs, gs);
  }
#endif

  if (exception_handlers[context->no] != 0) {
    interrupt_handler_t handler = exception_handlers[context->no];
    handler(context);
  }
}
INTERRUPT_SERVICE
void divide_error() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void debug_exception() {
  interrupt_entering_code(1, 0);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void nmi() {
  interrupt_entering_code(2, 0);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void breakpoint() {
  interrupt_entering_code(3, 0);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void overflow() {
  interrupt_entering_code(4, 0);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void bounds_check() {
  interrupt_entering_code(5, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void invalid_opcode() {
  interrupt_entering_code(6, 0);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void cop_not_avalid() {
  interrupt_entering_code(7, 0);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void double_fault() {
  interrupt_entering(8);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void overrun() {
  interrupt_entering_code(9, 0);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void invalid_tss() {
  interrupt_entering(10);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void seg_not_present() {
  interrupt_entering(11);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void stack_exception() {
  interrupt_entering(12);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void general_protection() {
  interrupt_entering(13);
  interrupt_process(exception_info);
  cpu_halt();
  interrupt_exit();
}

INTERRUPT_SERVICE void page_fault() {
  interrupt_entering(14);
  interrupt_process(exception_info);
  // cpu_halt();
  interrupt_exit();
}
INTERRUPT_SERVICE
void reversed() {
  interrupt_entering_code(15, 0);
  interrupt_process(exception_info);
  cpu_halt();
}
INTERRUPT_SERVICE
void coprocessor_error() {
  interrupt_entering_code(16, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

void do_page_fault(interrupt_context_t *context) {
#ifdef ARM

#elif defined(X86)
  u32 fault_addr;
  asm volatile("mov %%cr2, %0" : "=r"(fault_addr));

  int present = context->code & 0x1;  // present
  int rw = context->code & 0x2;       // rw
  int us = context->code & 0x4;       // user mode
  int reserved = context->code & 0x8;
  int id = context->code & 0x10;

  kprintf("eip:%x cs:%x ds:%x ss:%x esp:%x ebp:%x [", context->eip, context->cs,
          context->ds, context->ss, context->esp, context->ebp);
  if (present == 1) {
    kprintf("present ");
  }
  if (rw) {
    kprintf("read-only ");
  }
  if (us) {
    kprintf("user-mode ");
  }
  if (reserved) {
    kprintf("reserved ");
  }
  kprintf("] fault_addr:");
  kprintf("0x%x", fault_addr);

  if (present == 0) {
    thread_t *current = thread_current();
    if (fault_addr == NULL) {
      kprintf(" tid: %x warning null pointer\n", current->id);
      return;
    }
    if (current != NULL) {
      void *phy = virtual_to_physic(current->context.page_dir, fault_addr);
      kprintf(" tid: %x ", current->id);
      if (phy == NULL) {
        valloc(fault_addr, PAGE_SIZE);
      } else {
        kprintf("paddr: %x ", phy);
      }
    } else {
      map_page(fault_addr, fault_addr, PAGE_P | PAGE_USU | PAGE_RWW);
      kprintf("\n");
    }
  }
#endif
}

INTERRUPT_SERVICE
void reset_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void undefined_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void svc_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void pref_abort_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}


INTERRUPT_SERVICE
void data_abort_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}


INTERRUPT_SERVICE
void unuse_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}


void do_irq(interrupt_context_t* interrupt_context) {
  
}
extern context_t* current_context;

INTERRUPT_SERVICE
void irq_handler() {
  // interrupt_entering_code(0, 0);
  // interrupt_process(do_irq);
  // cpu_halt();
  interrupt_entering_code(ISR_TIMER,0);
  interrupt_process(do_schedule);
  //interrupt_exit();
  interrupt_exit_context(current_context);
}



INTERRUPT_SERVICE
void frq_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}


void exception_init() {
#ifdef ARM

  interrutp_regist(0, reset_handler);  // reset
  interrutp_regist(1, undefined_handler);  // undefined
  interrutp_regist(2, svc_handler);  // svc
  interrutp_regist(3, pref_abort_handler);  // pref abort
  interrutp_regist(4, data_abort_handler);  // data abort
  interrutp_regist(5, unuse_handler);  // not use
  interrutp_regist(6, irq_handler);  // irq
  interrutp_regist(7, frq_handler);  // fiq

#elif defined(X86)
  interrutp_regist(0, divide_error);
  interrutp_regist(1, debug_exception);
  interrutp_regist(2, nmi);
  interrutp_regist(3, breakpoint);
  interrutp_regist(4, overflow);
  interrutp_regist(5, bounds_check);
  interrutp_regist(6, invalid_opcode);
  interrutp_regist(7, cop_not_avalid);
  interrutp_regist(8, double_fault);
  interrutp_regist(9, overrun);
  interrutp_regist(10, invalid_tss);
  interrutp_regist(11, seg_not_present);
  interrutp_regist(12, stack_exception);
  interrutp_regist(13, general_protection);
  interrutp_regist(14, page_fault);
  interrutp_regist(15, reversed);
  interrutp_regist(16, coprocessor_error);
  interrutp_regist(17, reversed);
  interrutp_regist(18, reversed);

  // exception
  exception_regist(14, do_page_fault);

#endif
}
