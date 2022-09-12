/*******************************************************************
* Copyright 2021-2080 evilbinary
* 作者: evilbinary on 01/01/20
* 邮箱: rootdebug@163.com
********************************************************************/
#ifndef ARCH_BOOT_H
#define ARCH_BOOT_H

#include "libs/include/types.h"

#define MAX_MEMORY_BLOCK 10
#define MAX_CPU 4
#define CPU_NUMBER 2
#define GDT_NUMBER 12

#ifdef ARM

typedef u32 idt_entry_t;

#elif defined(X86)
#define GDT_SIZE 8

#define GDT_ENTRY_NULL 0
#define GDT_ENTRY_32BIT_CS 1
#define GDT_ENTRY_32BIT_DS 2
#define GDT_ENTRY_32BIT_FS 3

#define GDT_ENTRY_32BIT_TSS 4

#define GDT_ENTRY_USER_32BIT_CS 5
#define GDT_ENTRY_USER_32BIT_DS 6
#define GDT_ENTRY_USER_32BIT_FS 7


#define GDT_ENTRY_16BIT_CS 8
#define GDT_ENTRY_16BIT_DS 9
#define GDT_ENTRY_16BIT_FLAT_CS 10
#define GDT_ENTRY_16BIT_FLAT_DS 11

#define GDT_DPL(x) (((x)&0x03)<<5)  //0-3

#define GDT_NUM_ENTRIES 9

#define GDT_ENTRY(base, limit, flags)                                        \
  ((((base)&0xff000000ULL) << (56 - 24)) | (((flags)&0x0000f0ffULL) << 40) | \
   (((limit)&0x000f0000ULL) << (48 - 16)) | (((base)&0x00ffffffULL) << 16) | \
   (((limit)&0x0000ffffULL)))


typedef struct gdt_ptr {
  u16 limit;
  u32 base;
} __attribute__((packed)) gdt_ptr_t;

typedef struct idt_entry{
   u16 basel;
   u16 selector;
   u8  zero;
   u8  attrs;
   u16 baseh;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr {
  u16 limit;
  u32 base;
} __attribute__((packed)) idt_ptr_t;

#endif

typedef struct memory_info {
  u64 base;
  u64 length;
  u32 type;
  u32 extended;
} __attribute__((packed)) memory_info_t;

typedef struct memory_sinfo {
  u32 basel;   
  u32 baseh;
  u32 lengthl;
  u32 lengthh;
  u32 type;
  u32 extended;
} memory_sinfo_t;

typedef struct disply_info {
  i32 mode;
  u32* video;
  i32 width;
  i32 height;
} disply_info_t;

typedef struct disk_info{
  i32 hpc;
  i32 spt;
  i32 type;
}disk_info_t;


typedef struct tss_t {
  u32 back_link;
  u32 esp0, ss0;
  u32 esp1, ss1;
  u32 esp2, ss2;
  u32 cr3;
  u32 eip;
  u32 eflags;
  u32 eax, ecx, edx, ebx;
  u32 esp, ebp;
  u32 esi, edi;
  u32 es, cs, ss, ds, fs, gs;
  u32 ldt;
  u32 trace_bitmap;
} tss_t;

typedef struct boot_info {  
  i32 version;
  u32* kernel_origin_base;
  u32* kernel_base;
  u32* kernel_size;
  void* kernel_entry;
  u32* second_boot_entry;
  tss_t tss[MAX_CPU];

  i32 tss_number;
  u32* idt_base;
  i32 idt_number;
  
  u32* pdt_base;
  i32 page_type;
  
  u64 gdt_base[GDT_NUMBER];
  i32 gdt_number;

  disk_info_t disk;
  disply_info_t disply;

  i32 memory_number;
  i32 total_memory;
  memory_info_t memory[MAX_MEMORY_BLOCK];
} boot_info_t;

#endif