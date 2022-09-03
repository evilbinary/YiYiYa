/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "exceptions.h"

#include "thread.h"

interrupt_handler_t *exception_handlers[EXCEPTION_NUMBER];
void exception_regist(u32 vec, interrupt_handler_t handler) {
  exception_handlers[vec] = handler;
}

void exception_info(interrupt_context_t *context) {
  int cpu = cpu_get_id();
#ifdef ARM
#ifdef ARMV7
  static const char *exception_msg[] = {
      "NONE", "RESET", "NONE", "NONE", "NONE", "NONE", "NONE",       "NONE",
      "NONE", "NONE",  "NONE", "SVC",  "NONE", "NONE", "SYS PENDSV", "SYS TICK",
  };
  if (context->no < sizeof exception_msg) {
    kprintf("exception cpu %d no %d: %s\n----------------------------\n",cpu, context->no,
            exception_msg[context->no]);
  } else {
    kprintf("exception cpu %d no %d:\n----------------------------\n",cpu, context->no);
  }

#else
  static const char *exception_msg[] = {"RESET",      "UNDEFINED",  "SVC",
                                        "PREF ABORT", "DATA ABORT", "NOT USE",
                                        "IRQ",        "FIQ"};
  if (context->no < sizeof exception_msg) {
    kprintf("exception cpu %d no %d: %s\n----------------------------\n",cpu, context->no,
            exception_msg[context->no]);
  } else {
    kprintf("exception cpu %d no %d:\n----------------------------\n",cpu, context->no);
  }
  thread_t *current = thread_current();
  if (current != NULL) {
    kprintf("tid:%d %s cpu:%d\n", current->id, current->name,current->cpu_id);
  }
  kprintf("ifsr: %x dfsr: %x dfar: %x\n", read_ifsr(), read_dfsr(),
          read_dfar());
  kprintf("pc: %x\n", read_pc());
  context_dump_interrupt(context);
#endif

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
  //cls();
  if (context->no != 14) {
    thread_t *current = thread_current();
    if (context->no < sizeof exception_msg) {
      kprintf("exception cpu %d no %d: %s",cpu, context->no, exception_msg[context->no]);

    } else {
      kprintf("interrupt cpu %d %d", cpu,context->no);
    }
    if (current != NULL) {
      kprintf("\ntid %d %s cpu %d", current->id, current->name,current->cpu_id);
    }
    kprintf("\n----------------------------\n");
    context_dump_interrupt(context);
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

#ifdef ARM

#ifdef ARMV7

INTERRUPT_SERVICE
void reset_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void svc_handler() {
  interrupt_entering_code(2, 0);
  interrupt_process(exception_info);
  interrupt_exit();
}

INTERRUPT_SERVICE
void sys_tick_handler() {
  interrupt_entering_code(2, 0);
  interrupt_process(do_schedule);
  interrupt_exit_ret();
}

INTERRUPT_SERVICE
void sys_pendsv_handler() {
  interrupt_entering_code(2, 0);
  interrupt_process(exception_info);
  interrupt_exit();
}

#else

INTERRUPT_SERVICE
void reset_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void undefined_handler() {
  interrupt_entering_code(1, 0);
  interrupt_process(exception_info);
  interrupt_exit();
  // cpu_halt();
}

INTERRUPT_SERVICE
void svc_handler() {
  interrupt_entering_code(2, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void pref_abort_handler() {
  interrupt_entering_code(3, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void data_abort_handler() {
  interrupt_entering_code(4, 0);
  interrupt_process(do_page_fault);
  // cpu_halt();
  // interrupt_exit();
  interrupt_exit2();
}

INTERRUPT_SERVICE
void unuse_handler() {
  interrupt_entering_code(5, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

void do_irq(interrupt_context_t *interrupt_context) {}

INTERRUPT_SERVICE
void irq_handler() {
  // interrupt_entering_code(0, 0);
  // interrupt_process(do_irq);
  // cpu_halt();
  // interrupt_exit();
  interrupt_entering_code(ISR_TIMER, 0);
  interrupt_process(do_schedule);
  interrupt_exit_ret();
}

INTERRUPT_SERVICE
void frq_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

void do_page_fault(interrupt_context_t *context) {
  u32 fault_addr = read_dfar();
  thread_t *current = thread_current();
  if (current != NULL) {
    int mode = context_get_mode(&current->context);
    // kprintf("mode %x\n", mode);
    if (mode == USER_MODE) {
      vmemory_area_t *area = vmemory_area_find(current->vmm, fault_addr, 0);
      if (area == NULL) {
        if (current->fault_count < 3) {
          thread_exit(current, -1);
          kprintf("tid: %d %s memory fault at %x\n", current->id, current->name,
                  fault_addr);
          dump_fault(context, fault_addr);
          current->fault_count++;
        } else if (current->fault_count == 3) {
          kprintf("tid: %d %s memory fault at %x too many\n",
                  current->id, current->name, fault_addr);
          current->fault_count++;
          thread_exit(current, -1);
        }
        
        return;
      }
      // kprintf("exception at %x\n",page_fault);
      void *phy = virtual_to_physic(current->context.page_dir, fault_addr);
      if (phy == NULL) {
        valloc(fault_addr, PAGE_SIZE);
      } else {
        // valloc(fault_addr, PAGE_SIZE);
        kprintf("tid: %d %s phy: %x remap memory fault at %x\n", current->id,
                current->name, phy, fault_addr);
        dump_fault(context, fault_addr);
        // mmu_dump_page(current->context.page_dir,current->context.page_dir,0);
        thread_exit(current, -1);
        cpu_halt();
      }
    } else {
      kprintf("tid: %d kernel memory fault at %x\n", current->id, fault_addr);
      map_page(fault_addr, fault_addr, PAGE_P | PAGE_USU | PAGE_RWW);
    }
  } else {
    map_page(fault_addr, fault_addr, PAGE_P | PAGE_USU | PAGE_RWW);
  }
}

void dump_fault(interrupt_context_t *context, u32 fault_addr) {
  kprintf("----------------------------\n");
  kprintf("ifsr: %x dfsr: %x dfar: %x\n", read_ifsr(), read_dfsr(),
          read_dfar());
  kprintf("pc: %x\n", read_pc());
  context_dump_interrupt(context);
  kprintf("fault: 0x%x \n", fault_addr);
  kprintf("----------------------------\n\n");
}

#endif

#elif defined(X86)

void dump_fault(interrupt_context_t *context, u32 fault_addr) {
  int present = context->code & 0x1;  // present
  int rw = context->code & 0x2;       // rw
  int us = context->code & 0x4;       // user mode
  int reserved = context->code & 0x8;
  int id = context->code & 0x10;
  kprintf("----------------------------\n");
  context_dump_interrupt(context);
  kprintf("page: [");
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
  kprintf("]\n");
  kprintf("fault: 0x%x \n", fault_addr);
  kprintf("----------------------------\n");
}
void do_page_fault(interrupt_context_t *context) {
  u32 fault_addr;
  asm volatile("mov %%cr2, %0" : "=r"(fault_addr));
  int present = context->code & 0x1;

  if (present == 0) {
    thread_t *current = thread_current();
    if (current != NULL) {
      vmemory_area_t *area = vmemory_area_find(current->vmm, fault_addr, 0);
      if (area == NULL) {
        if(current->state==THREAD_STOPPED){
          return;
        }
        if (current->fault_count < 3) {
          thread_exit(current, -1);
          kprintf("tid: %d %s memory fault at %x\n", current->id, current->name,
                  fault_addr);
          dump_fault(context, fault_addr);
          current->fault_count++;
        } else if (current->fault_count == 3) {
          kprintf("tid: %d %s memory fault at %x too many\n",
                  current->id, current->name, fault_addr);
          current->fault_count++;
          thread_exit(current, -1);
        }
        return;
      }
      void *phy = virtual_to_physic(current->context.page_dir, fault_addr);

#ifdef DEBUG_EXCEPTION
      kprintf(" tid: %x ", current->id);
#endif
      if (phy == NULL) {
        valloc(fault_addr, PAGE_SIZE);
      } else {
        kprintf("tid: %d %s phy remap memory fault at %x\n", current->id,
                current->name, fault_addr);
        dump_fault(context, fault_addr);
        thread_exit(current, -1);
      }
    } else {
      map_page(fault_addr, fault_addr, PAGE_P | PAGE_USU | PAGE_RWW);
    }
  }
}
#elif defined(XTENSA)
INTERRUPT_SERVICE
void reset_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void l1_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void l2_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void l3_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void l4_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void l5_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void debug_excetpion_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void nmi_excetpion_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void kernel_excetpion_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void user_excetpion_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}

INTERRUPT_SERVICE
void double_excetpion_handler() {
  interrupt_entering_code(0, 0);
  interrupt_process(exception_info);
  cpu_halt();
}
#endif

void exception_init() {
#ifdef ARM

#ifdef ARMV7
  interrutp_regist(1, reset_handler);        // reset
  interrutp_regist(11, svc_handler);         // svc_handler
  interrutp_regist(14, sys_pendsv_handler);  // sys_pendsv_handler
  interrutp_regist(15, sys_tick_handler);    // sys_tick_handler

#else
  interrutp_regist(0, reset_handler);       // reset
  interrutp_regist(1, undefined_handler);   // undefined
  interrutp_regist(2, svc_handler);         // svc
  interrutp_regist(3, pref_abort_handler);  // pref abort
  interrutp_regist(4, data_abort_handler);  // data abort
  interrutp_regist(5, unuse_handler);       // not use
  interrutp_regist(6, irq_handler);         // irq
  interrutp_regist(7, frq_handler);         // fiq
#endif

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

#elif defined(XTENSA)
  interrutp_regist(0, reset_handler);  // reset
  interrutp_regist(6, l2_handler);
  interrutp_regist(7, l3_handler);
  interrutp_regist(8, l4_handler);
  interrutp_regist(9, l5_handler);

  interrutp_regist(10, debug_excetpion_handler);
  interrutp_regist(11, nmi_excetpion_handler);
  interrutp_regist(12, kernel_excetpion_handler);
  interrutp_regist(15, user_excetpion_handler);
  interrutp_regist(16, double_excetpion_handler);

#endif
}
