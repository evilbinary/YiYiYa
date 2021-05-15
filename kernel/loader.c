/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "loader.h"

#include "fd.h"
#include "kernel.h"
#include "kernel/elf.h"
#include "thread.h"
#include "vfs.h"

#define DEBUG

void* load_elf(Elf32_Ehdr* elf_header, u32 fd, page_dir_t* page) {
  // printf("e_phnum:%d\n\r", elf_header->e_phnum);
  u32 offset = elf_header->e_phoff;
  Elf32_Phdr* phdr = syscall0(SYS_VHEAP);
  syscall2(SYS_SEEK, fd, offset);
  u32 nbytes =
      syscall3(SYS_READ, fd, phdr, sizeof(Elf32_Phdr) * elf_header->e_phnum);
  // printf("addr %x elf=%x\n\r", phdr, elf);
  u32 entry = 0;
  for (int i = 0; i < elf_header->e_phnum; i++) {
#ifdef DEBUG
    kprintf("type:%d\n\r", phdr[i].p_type);
#endif
    switch (phdr[i].p_type) {
      case PT_NULL:
        // kprintf(" %s %x %x %x %s %x %x \r\n", "NULL", phdr[i].p_offset,
        //        phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
        //        phdr[i].p_memsz);
        break;
      case PT_LOAD: {
#ifdef DEBUG
        kprintf(" %s %x %x %x %s %x %x \r\n", "LOAD", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
#endif
        char* start = phdr[i].p_offset;
        char* vaddr = phdr[i].p_vaddr;
        syscall2(SYS_SEEK, fd, start);
        entry = vaddr;
        u32 ret = syscall3(SYS_READ, fd, vaddr, phdr[i].p_filesz);
      } break;
      // case PT_DYNAMIC:
      //   printf(" %s %x %x %x\r\n %s %x %x ",
      //          "DYNAMIC", phdr[i].p_offset, phdr[i].p_vaddr,
      //          phdr[i].p_paddr, "", phdr[i].p_filesz,
      //          phdr[i].p_memsz);
      //   break;
      // case PT_INTERP:
      //   printf(" %s %x %x %x\r\n %s %x %x ",
      //          "INTERP", phdr[i].p_offset, phdr[i].p_vaddr,
      //          phdr[i].p_paddr, "", phdr[i].p_filesz,
      //          phdr[i].p_memsz);
      //   break;
      // case PT_NOTE:
      //   printf(" %s %x %x %x\r\n %s %x %x ", "NOTE",
      //          phdr[i].p_offset, phdr[i].p_vaddr,
      //          phdr[i].p_paddr, "", phdr[i].p_filesz,
      //          phdr[i].p_memsz);
      //   break;
      // case PT_SHLIB:
      //   printf(" %s %x %x %x\r\n %s %x %x ",
      //          "SHLIB", phdr[i].p_offset, phdr[i].p_vaddr,
      //          phdr[i].p_paddr, "", phdr[i].p_filesz,
      //          phdr[i].p_memsz);
      //   break;
      // case PT_PHDR:
      //   printf(" %s %x %x %x\r\n %s %x %x ", "PHDR",
      //          phdr[i].p_offset, phdr[i].p_vaddr,
      //          phdr[i].p_paddr, "", phdr[i].p_filesz,
      //          phdr[i].p_memsz);
      //   break;
      // case PT_TLS:
      //   printf(" %s %x %x %x\r\n %s %x %x ", "TLS",
      //          phdr[i].p_offset, phdr[i].p_vaddr,
      //          phdr[i].p_paddr, "", phdr[i].p_filesz,
      //          phdr[i].p_memsz);
      //   break;
      // case PT_NUM:
      //   printf(" %s %x %x %x\r\n %s %x %x ", "NUM",
      //          phdr[i].p_offset, phdr[i].p_vaddr,
      //          phdr[i].p_paddr, "", phdr[i].p_filesz,
      //          phdr[i].p_memsz);
      //   break;
      // case PT_GNU_EH_FRAME:
      //   printf(" %s %x %x %x\r\n %s %x %x ",
      //          "GNU_EH_FRAME", phdr[i].p_offset, phdr[i].p_vaddr,
      //          phdr[i].p_paddr, "", phdr[i].p_filesz,
      //          phdr[i].p_memsz);
      //   break;
      // case PT_GNU_RELRO:
      //   printf(" %s %x %x %x\r\n %s %x %x ",
      //          "GNU_RELRO", phdr[i].p_offset, phdr[i].p_vaddr,
      //          phdr[i].p_paddr, "", phdr[i].p_filesz,
      //          phdr[i].p_memsz);
      //   break;
      // case PT_GNU_STACK:
      //   printf(" %s %x %x %x\r\n %s %x %x ",
      //          "GNU_STACK", phdr[i].p_offset, phdr[i].p_vaddr,
      //          phdr[i].p_paddr, "", phdr[i].p_filesz,
      //          phdr[i].p_memsz);
      //   break;
      default:
        break;
    }
  }
  // data
  offset = elf_header->e_shoff;
  Elf32_Shdr* shdr = syscall0(SYS_VHEAP);
  syscall2(SYS_SEEK, fd, offset);
  nbytes =
      syscall3(SYS_READ, fd, shdr, sizeof(Elf32_Shdr) * elf_header->e_shnum);
  for (int i = 0; i < elf_header->e_shnum; i++) {
    if (SHT_NOBITS == shdr[i].sh_type) {
      char* vaddr = shdr[i].sh_addr;
      // map_alignment(page,vaddr,buf,shdr[i].sh_size);
    } else if (elf_header->e_entry != shdr[i].sh_addr &&
               SHT_PROGBITS == shdr[i].sh_type &&
               shdr[i].sh_flags & SHF_ALLOC && shdr[i].sh_flags) {
      char* start = shdr[i].sh_offset;
      char* vaddr = shdr[i].sh_addr;
#ifdef DEBUG
      kprintf("load start:%x vaddr:%x size:%x \n\r", start, vaddr,
              phdr[i].p_filesz);
#endif
      syscall2(SYS_SEEK, fd, start);
      u32 ret = syscall3(SYS_READ, fd, vaddr, shdr[i].sh_size);
      // map_alignment(page,vaddr,buf,shdr[i].sh_size);
    }
  }
  return entry;
}

void run_elf_thread() {
  Elf32_Ehdr elf;
  thread_t* current = thread_current();
  exec_t* exec = current->data;
  u32 fd = syscall2(SYS_OPEN, exec->filename, 0);
  kprintf("load elf %s fd:%d\n", exec->filename, fd);
  if (fd < 0) {
    kprintf("open elf %s error\n", exec->filename);
    return;
  }

  u32 nbytes = syscall3(SYS_READ, fd, &elf, sizeof(Elf32_Ehdr));
  Elf32_Ehdr* elf_header = (Elf32_Ehdr*)&elf;
  u32* kentry = 0;
  entry_fn entry = NULL;
  if (elf_header->e_ident[0] == ELFMAG0 || elf_header->e_ident[1] == ELFMAG1) {
    kentry = load_elf(elf_header, fd, current->context.page_dir);
    entry = elf_header->e_entry;
  } else {
    kprintf("load faild not elf %s\n", exec->filename);
  }
  // map_page_on(current->context.page_dir,current->context.esp,current->context.esp,PAGE_P
  // | PAGE_USU | PAGE_RWW);
  u32 ret = -1;
  if (entry != NULL) {
#ifdef DEBUG
    kprintf("entry %x\n", entry);
#endif
    ret = entry(0, exec->argv);
  }
  thread_stop(current);
  // syscall1(SYS_EXIT, ret);
  // kprintf("wait stop tid:%d state:%d\n",current->id,current->state);
  for (;;) {
    // kprintf(".");
  }
}
