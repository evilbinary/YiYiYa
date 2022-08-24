/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "../cpu.h"

extern boot_info_t* boot_info;


void context_init(context_t* context, u32* entry, u32* stack0, u32* stack3,
                  u32 level,int cpu) {
  if (context == NULL) {
    return;
  }
  tss_t* tss = &boot_info->tss[cpu];
  context->tss = tss;
  context->eip = entry;
  context->level = level;
  u32 cs, ds;
  if (level == 0) {
    cs = GDT_ENTRY_32BIT_CS * GDT_SIZE;
    ds = GDT_ENTRY_32BIT_DS * GDT_SIZE;
  } else if (level == 3) {
    cs = GDT_ENTRY_USER_32BIT_CS * GDT_SIZE | level;
    ds = GDT_ENTRY_USER_32BIT_DS * GDT_SIZE | level;
  } else {
    kprintf("not suppport level %d\n", level);
  }
  interrupt_context_t* c = stack0;
  c->ss = ds;          // ss
  c->esp = stack3;     // esp
  c->eflags = 0x0200;  // eflags
  c->cs = cs;          // cs
  c->eip = entry;      // eip 4

  c->no = 0;             // no  5
  c->code = 0;           // no  5
  c->eax = 0;            // eax 6
  c->ecx = 0;            // ecx 7
  c->edx = 0;            // edx 8
  c->ebx = 0;            // ebx 9
  c->esp_null = stack0;  // esp 10
  c->ebp = stack3;       // ebp 11
  c->esi = 0;            // esi 12
  c->edi = 0;            // edi 13
  c->ds = ds;            // ds  14
  c->es = ds;            // es  15
  c->fs = ds;            // fs  16
  c->gs = ds;            // gs    17

  context->esp0 = c;
  context->ss0 = GDT_ENTRY_32BIT_DS * GDT_SIZE;
  context->ds0 = GDT_ENTRY_32BIT_DS * GDT_SIZE;
  context->esp = stack3;
  context->ss = ds;
  context->ds = ds;

  ulong addr = (ulong)boot_info->pdt_base;
  context->page_dir = addr;
  context->kernel_page_dir = addr;

  if (tss->eip == 0 && tss->cr3 == 0) {
    tss->ss0 = context->ss0;
    tss->esp0 = context->esp0 + sizeof(interrupt_context_t);
    tss->eip = context->eip;
    tss->ss = ds;
    tss->ds = tss->es = tss->fs = tss->gs = ds;
    tss->cs = cs;
    tss->esp = context->esp;
    tss->cr3 = boot_info->pdt_base;
    set_ldt(GDT_ENTRY_32BIT_TSS * GDT_SIZE);
  }
}

int context_get_mode(context_t* context) {
  int mode = 0;
  if (context != NULL) {
  }
  return mode;
}

void cpu_backtrace(stack_frame_t* fp, void** buf, int size) {
  int i;
  for (i = 0; i < size && fp != NULL; fp = fp->prev, i++) {
    buf[i] = fp->return_addr;
  }
}

void context_dump(context_t* c) {
  kprintf("eip:     %x\n", c->eip);
  kprintf("esp0:    %x\n", c->esp0);
  kprintf("esp:     %x\n", c->esp);

  kprintf("page_dir: %x\n", c->page_dir);
  kprintf("kernel page_dir: %x\n", c->kernel_page_dir);

  if (c->esp0 != 0) {
    context_dump_interrupt(c->esp0);
  }
}

void context_dump_interrupt(interrupt_context_t* context) {
  u32 cr2, cr3;
  u32 ds, es, fs, gs;
  asm volatile("movl	%%cr2,	%%eax" : "=a"(cr2));
  asm volatile("movl %%cr3,	%%eax" : "=a"(cr3));
  asm volatile("movl %%ds,	%%eax" : "=a"(ds));
  asm volatile("movl %%es,	%%eax" : "=a"(es));
  asm volatile("movl %%fs,	%%eax" : "=a"(fs));
  asm volatile("movl %%gs,	%%eax" : "=a"(gs));
  kprintf("cs:\t%x\teip:\t%x\teflags:\t%x\n", context->cs, context->eip,
          context->eflags);
  kprintf("ss:\t%x\tesp:\t%x\n", context->ss, context->esp);
  // kprintf("old ss:\t%x\told esp:%x\n", old_ss, old_esp);
  kprintf("code:%x\tcr2:\t%x\tcr3:\t%x\n", context->no, cr2, cr3);
  kprintf("General Registers:\n----------------------------\n");
  kprintf("eax:\t%x\tebx:\t%x\n", context->eax, context->ebx);
  kprintf("ecx:\t%x\tedx:\t%x\n", context->ecx, context->edx);
  kprintf("esi:\t%x\tedi:\t%x\tebp:\t%x\n", context->esi, context->edi,
          context->ebp);
  kprintf("Segment Registers:\n----------------------------\n");
  kprintf("ds:\t%x\tes:\t%x\n", ds, es);
  kprintf("fs:\t%x\tgs:\t%x\n", fs, gs);
  // char* buf[10]; 
  // stack_frame_t fp;
  // fp.prev=context->eip;
  // fp.return_addr=context->eip;
  // cpu_backtrace(&fp,buf,10);
  // kprintf("backtrace:\n");
  // for(int i=0;i<10;i++){
  //   kprintf("   %x\n",buf[i]);
  // }
}

void context_clone(context_t* des, context_t* src, u32* stack0, u32* stack3,
                   u32* old0, u32* old3) {
  *des = *src;
  interrupt_context_t* c0 = stack0;
  interrupt_context_t* s0 = src->esp0;
  if (stack0 != NULL) {
    *c0 = *s0;
    c0->eflags = 0x0200;
  }
  if (stack3 != NULL) {
    des->esp = s0->esp;
  }
  des->esp0 = (u32)c0;

  des->page_dir = page_alloc_clone(src->page_dir);
}

void context_switch(interrupt_context_t* context, context_t** current,
                    context_t* next_context) {
  context_t* current_context = *current;

  if (context == NULL) {
    context = current_context->esp0;
    context->esp = current_context->esp;
  } else {
    current_context->esp0 = context;
    current_context->esp = context->esp;
  }

  interrupt_context_t* c = next_context->esp0;

  tss_t* tss = next_context->tss;
  tss->esp0 = next_context->esp0 + sizeof(interrupt_context_t);
  tss->ss0 = next_context->ss0;
  tss->cr3 = next_context->page_dir;

  *current = next_context;
  context_switch_page(next_context->page_dir);
}
