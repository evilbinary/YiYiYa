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

int load_elf(Elf32_Ehdr* elf_header, u32 fd) {
  u32 offset = elf_header->e_phoff;
  if (elf_header->e_phnum > MAX_PHDR) {
    kprintf("phnum %d > MAX_PHDR\n", elf_header->e_phnum);
    return -1;
  }
  Elf32_Phdr phdr[MAX_PHDR];
  kmemset(phdr, 0, MAX_PHDR * sizeof(Elf32_Phdr));
  syscall3(SYS_SEEK, fd, offset,0);
  u32 nbytes =
      syscall3(SYS_READ, fd, phdr, sizeof(Elf32_Phdr) * elf_header->e_phnum);
  // kprintf("addr %x elf=%x\n\r", phdr, elf);
  u32 entry = elf_header->e_entry;
  u32 entry_txt = 0;
  for (int i = 0; i < elf_header->e_phnum; i++) {
#ifdef LOAD_ELF_DEBUG
    kprintf("ptype:%d\n\r", phdr[i].p_type);
#endif
    switch (phdr[i].p_type) {
      case PT_NULL:
        kprintf(" %s %x %x %x %s %x %x \r\n", "NULL", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
        break;
      case PT_LOAD: {
        if ((phdr[i].p_flags & PF_X) == PF_X) {
#ifdef LOAD_ELF_DEBUG
          kprintf(" %s %x %x %x %s %x %x \r\n", "LOAD", phdr[i].p_offset,
                  phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                  phdr[i].p_memsz);
#endif

          char* start = phdr[i].p_offset;
          char* vaddr = phdr[i].p_vaddr;
          syscall3(SYS_SEEK, fd, start,0);
          entry_txt = vaddr;
          u32 ret = syscall3(SYS_READ, fd, vaddr, phdr[i].p_filesz);
        } else {
#ifdef LOAD_ELF_DEBUG
          kprintf(" %s %x %x %x %s %x %x \r\n", "NO LOAD", phdr[i].p_offset,
                  phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                  phdr[i].p_memsz);
#endif
        }
      } break;
      case PT_DYNAMIC:
        kprintf(" %s %x %x %x\r\n %s %x %x ", "DYNAMIC", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
        break;
      case PT_INTERP:
        kprintf(" %s %x %x %x\r\n %s %x %x ", "INTERP", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
        break;
      case PT_NOTE:
        kprintf(" %s %x %x %x\r\n %s %x %x ", "NOTE", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
        break;
      case PT_SHLIB:
        kprintf(" %s %x %x %x\r\n %s %x %x ", "SHLIB", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
        break;
      case PT_PHDR:
        kprintf(" %s %x %x %x\r\n %s %x %x ", "PHDR", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
        break;
      case PT_TLS:
        kprintf(" %s %x %x %x\r\n %s %x %x ", "TLS", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
        break;
      case PT_NUM:
        kprintf(" %s %x %x %x\r\n %s %x %x ", "NUM", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
        break;
      case PT_GNU_EH_FRAME:
        kprintf(" %s %x %x %x\r\n %s %x %x ", "GNU_EH_FRAME", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
        break;
      case PT_GNU_RELRO:
        kprintf(" %s %x %x %x\r\n %s %x %x ", "GNU_RELRO", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
        break;
      case PT_GNU_STACK:
        kprintf(" %s %x %x %x\r\n %s %x %x ", "GNU_STACK", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz);
        break;
      default:
        break;
    }
  }
  // data
  offset = elf_header->e_shoff;
  if (elf_header->e_shnum > MAX_SHDR) {
    kprintf("shnum %d > MAX_SHDR %d\n", elf_header->e_shnum, MAX_SHDR);
    return -1;
  }
  Elf32_Shdr shdr[MAX_SHDR];
  kmemset(shdr, 0, sizeof(Elf32_Shdr) * elf_header->e_shnum);
  syscall3(SYS_SEEK, fd, offset,0);
  nbytes =
      syscall3(SYS_READ, fd, shdr, sizeof(Elf32_Shdr) * elf_header->e_shnum);

  for (int i = 0; i < elf_header->e_shnum; i++) {
    if (SHT_NOBITS == shdr[i].sh_type) {
      char* vaddr = shdr[i].sh_addr;
      char* start = shdr[i].sh_offset;
      // kmemset(vaddr, 0, shdr[i].sh_size);
#ifdef LOAD_ELF_DEBUG
      kprintf("NOBITS start:%x vaddr:%x sh_size:%x \n\r", start, vaddr,
              shdr[i].sh_size);
#endif
      // map_alignment(page,vaddr,buf,shdr[i].sh_size);
    } else if ((shdr[i].sh_type&SHT_PROGBITS == SHT_PROGBITS) &&
              (shdr[i].sh_flags & SHF_ALLOC==SHF_ALLOC) && (shdr[i].sh_flags & SHF_WRITE==SHF_WRITE) ) {
      char* start = shdr[i].sh_offset;
      char* vaddr = shdr[i].sh_addr;
#ifdef LOAD_ELF_DEBUG
      kprintf("data start:%x vaddr:%x sh_size:%x \n\r", start, vaddr,
              shdr[i].sh_size);
#endif
      syscall3(SYS_SEEK, fd, start,0);
      u32 ret = syscall3(SYS_READ, fd, vaddr, shdr[i].sh_size);
      // map_alignment(page,vaddr,buf,shdr[i].sh_size);
    }
  }
  return 0;
}

void run_elf_thread() {
  Elf32_Ehdr elf;
  thread_t thread;
  thread_t* current=&thread;
  u32 ret=syscall1(SYS_SELF,current);
  if(ret<0){
    kprintf("get current thread error\n");
    return ;
  }
  exec_t* exec = current->exec;
  u32 fd = syscall2(SYS_OPEN, exec->filename, 0);
  #ifdef LOAD_ELF_NAME_DEBUG
  kprintf("load elf %s fd:%d tid:%d\n", exec->filename, fd, current->id);
  #endif
  if (fd < 0) {
    kprintf("open elf %s error\n", exec->filename);
    syscall1(SYS_EXIT, -1);
    return;
  }
  u32 nbytes = syscall3(SYS_READ, fd, &elf, sizeof(Elf32_Ehdr));

  Elf32_Ehdr* elf_header = (Elf32_Ehdr*)&elf;
  entry_fn entry = NULL;
  if (elf_header->e_ident[0] == ELFMAG0 || elf_header->e_ident[1] == ELFMAG1) {
    entry = elf_header->e_entry;
    load_elf(elf_header, fd);
  } else {
    kprintf("load faild not elf %s\n", exec->filename);
    entry = NULL;
  }
  ret = -1;
  if (entry != NULL) {
#ifdef LOAD_ELF_DEBUG
    kprintf("entry %x\n", entry);
#endif
    ret = entry(exec->argc, exec->argv, exec->envp);
  }
  syscall1(SYS_EXIT, ret);
}
