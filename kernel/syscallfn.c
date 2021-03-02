/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "syscallfn.h"

#include "kernel.h"
#include "kernel/elf.h"
#include "thread.h"
#include "vfs.h"

fd_t open_list[256];
int open_number = 0;

fd_t* find_fd(u32 fd) {
  for (int i = 0; i < open_number; i++) {
    if (open_list[i].id == fd) {
      return &open_list[i];
    }
  }
  return NULL;
}

int sys_print(char* s) {
  kprintf("%s", s);
  return 0;
}

void sys_test() { kprintf("sys test\n"); }

int sys_print_at(char* s, u32 x, u32 y) {
  set_cursor(x, y);
  kprintf("%s", s);
  return 0;
}

size_t sys_ioctl(u32 fd, u32 cmd, ...) {
  u32 ret = 0;
  fd_t* f = find_fd(fd);
  if (f == NULL) {
    kprintf("read not found fd %d\n", fd);
    return 0;
  }
  va_list args;
  va_start(args, cmd);
  vnode_t* node = f->data;
  ret = vioctl(node, cmd, args);
  va_end(args);
  return ret;
}

u32 sys_open(char* name, int attr) {
  // mm_dump();
  vnode_t* node = vfind(NULL, name);
  if (node == NULL) {
    kprintf("open %s failed \n", name);
    return -1;
  }
  char dup_name[256] = {0};
  kstrcpy(dup_name, name);
  char* token = kstrtok(dup_name, node->name);
  char* last;
  while (token != NULL) {
    last = token;
    token = kstrtok(NULL, node->name);
  }
  if (last[0] == '/') {
    last++;
  }
  vnode_t* file = vfind(node, last);
  if (file == NULL) {
    kprintf("find %s failed\n", token);
    return -1;
  }
  // u32 ret = vopen(file);
  if (open_number > 256) {
    kprintf("open limit \n");
    return -1;
  }
  open_list[open_number].id = open_number;
  open_list[open_number].type = DEVICE_TYPE_FILE;
  open_list[open_number].data = file;
  open_list[open_number].offset = 0;
  return open_list[open_number++].id;
}

void sys_close(u32 fd) {}

size_t sys_write(u32 fd, void* buf, size_t nbytes) {
  fd_t* f = find_fd(fd);
  if (f == NULL) {
    kprintf("write not found fd %d\n", fd);
    return 0;
  }
  vnode_t* node = f->data;
  u32 ret = vwrite(node, f->offset, nbytes, buf);
  return ret;
}
size_t sys_read(u32 fd, void* buf, size_t nbytes) {
  fd_t* f = find_fd(fd);
  if (f == NULL) {
    kprintf("read not found fd %d\n", fd);
    return 0;
  }
  vnode_t* node = f->data;
  u32 ret = vread(node, f->offset, nbytes, buf);
  return ret;
}

size_t sys_seek(u32 fd, u32 offset) {
  fd_t* f = find_fd(fd);
  if (f == NULL) {
    kprintf("read not found fd %d\n", fd);
    return 0;
  }
  f->offset = offset;
  return 1;
}

size_t sys_yeild(thread_t* thread) {
  if (thread != NULL) {
    thread->counter++;
    schedule();
  }
}

void* load_elf(Elf32_Ehdr* elf_header, u32 fd, page_dir_t* page) {
  // printf("e_phnum:%d\n\r", elf_header->e_phnum);
  u32 offset = elf_header->e_phoff;
  Elf32_Phdr* phdr = kmalloc(sizeof(Elf32_Phdr) * elf_header->e_phnum);
  sys_seek(fd, offset);
  u32 nbytes = sys_read(fd, phdr, sizeof(Elf32_Phdr));
  // printf("addr %x elf=%x\n\r", phdr, elf);
  u32 entry = 0;
  for (int i = 0; i < elf_header->e_phnum; i++) {
    // kprintf("type:%d\n\r", phdr[i].p_type);
    switch (phdr[i].p_type) {
      case PT_NULL:
        // printf(" %s %x %x %x %s %x %x \r\n", "NULL", phdr[i].p_offset,
        //        phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
        //        phdr[i].p_memsz);
        break;
      case PT_LOAD: {
        // printf(" %s %x %x %x %s %x %x \r\n", "LOAD", phdr[i].p_offset,
        //        phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
        //        phdr[i].p_memsz);
        char* start = phdr[i].p_offset;
        char* vaddr = phdr[i].p_vaddr;
        // printf("load start:%x vaddr:%x size:%x \n\r", start, vaddr,
        //        phdr[i].p_filesz);
        sys_seek(fd, start);
        char* buf = mm_alloc_zero_align(phdr[i].p_filesz, phdr[i].p_align);
        // char* buf = kmalloc(phdr[i].p_filesz);
        entry = buf;
        u32 ret = sys_read(fd, buf, phdr[i].p_filesz);
        u32 file_4k = 0x1000;
        if (phdr[i].p_filesz > 0x1000) {
          file_4k = phdr[i].p_filesz;
        }
        // bug to fix
        kmemmove(buf, buf + 10, phdr[i].p_filesz);
        for (int i = 0; i < file_4k / 0x1000; i++) {
          kprintf("map vaddr:%x paddr:%x\n", vaddr, buf);
          map_page_on(page, (u32)vaddr, (u32)buf, PAGE_P | PAGE_USU | PAGE_RWW);
          vaddr += 0x1000;
          buf += 0x1000;
        }

        // kmemmove(vaddr+nbytes, buf, ret);
        // kprintf("map end\n");
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
  // data section
  offset = elf_header->e_shoff;
  Elf32_Shdr* shdr = kmalloc(sizeof(Elf32_Shdr) * elf_header->e_shnum);
  sys_seek(fd, offset);
  nbytes = sys_read(fd, shdr, sizeof(Elf32_Shdr) * elf_header->e_shnum);
  for (int i = 0; i < elf_header->e_shnum; i++) {
    if (SHT_NOBITS == shdr[i].sh_type) {
      char* buf = kmalloc(shdr[i].sh_size);
      char* vaddr = shdr[i].sh_addr;
      map_page_on(page, (u32)vaddr, (u32)buf, PAGE_P | PAGE_USU | PAGE_RWW);
    } else if (elf_header->e_entry!=shdr[i].sh_addr&& SHT_PROGBITS == shdr[i].sh_type &&
               shdr[i].sh_flags & SHF_ALLOC&& shdr[i].sh_flags) {
      char* start = shdr[i].sh_offset;
      char* vaddr = shdr[i].sh_addr;
      // kprintf("load start:%x vaddr:%x size:%x \n\r", start, vaddr,
      //        phdr[i].p_filesz);
      sys_seek(fd, start);
      char* buf = mm_alloc_zero_align(shdr[i].sh_size, shdr[i].sh_addralign);
      // char* buf = kmalloc(phdr[i].p_filesz);
      entry = buf;
      u32 ret = sys_read(fd, buf, shdr[i].sh_size);
      u32 file_4k = 0x1000;
      if (shdr[i].sh_size > 0x1000) {
        file_4k = shdr[i].sh_size;
      }
      // bug to fix
      //kmemmove(buf, buf + 10, shdr[i].sh_size);
      for (int i = 0; i < file_4k / 0x1000; i++) {
        kprintf("map vaddr:%x paddr:%x\n", vaddr, buf);
        map_page_on(page, (u32)vaddr, (u32)buf, PAGE_P | PAGE_USU | PAGE_RWW);
        vaddr += 0x1000;
        buf += 0x1000;
      }
      break;
    }
  }

  return entry;
}

void map_2gb(u64* page_dir_ptr_tab, u64* page_dir, u64* page_tab) {
  // set zero
  for (int i = 0; i < 4; i++) {
    page_dir_ptr_tab[i] = 0;
  }
  page_dir_ptr_tab[0] = (u32)page_dir | PAGE_P;

  page_dir[0] = (u32)page_tab | (PAGE_P | PAGE_USU | PAGE_RWW);
  unsigned int i, address = 0;
  kprintf("start 0x%x ", address);
  for (i = 0; i < 512; i++) {
    page_tab[i] = address | (PAGE_P | PAGE_USU | PAGE_RWW);
    address = address + 0x1000;
  }
  kprintf("- 0x%x\n", address);  // 0x200000 2GB
}

void page_clone(u64* page, u64* page_dir_ptr_tab) {
  for (int pdpte_index = 0; pdpte_index < 4; pdpte_index++) {
    u64* page_dir_ptr = page[pdpte_index] & ~0xFFF;
    if (page_dir_ptr != NULL) {
      u64* new_page_dir_ptr = mm_alloc_zero_align(sizeof(u64) * 512, 0x1000);
      page_dir_ptr_tab[pdpte_index] =
          ((u64)new_page_dir_ptr) | PAGE_P | PAGE_USU | PAGE_RWW;
      for (int pde_index = 0; pde_index < 512; pde_index++) {
        u64* page_tab_ptr = (u64)page_dir_ptr[pde_index] & ~0xFFF;
        if (page_tab_ptr != NULL) {
          u64* new_page_tab_ptr =
              mm_alloc_zero_align(sizeof(u64) * 512, 0x1000);
          new_page_dir_ptr[pde_index] =
              ((u64)new_page_tab_ptr) | PAGE_P | PAGE_USU | PAGE_RWW;
          for (int pte_index = 0; pte_index < 512; pte_index++) {
            u64* page_tab_item = page_tab_ptr[pte_index] & ~0xFFF;
            if (page_tab_item != NULL) {
              new_page_tab_ptr[pte_index] = page_tab_ptr[pte_index];
              u32 virtualaddr = (pdpte_index & 0x03) << 30 |
                                (pde_index & 0x01FF) << 21 |
                                (pte_index & 0x01FF) << 12;
              // kprintf("map vir:%x phy:%x \n", virtualaddr,
              // new_page_tab_ptr[pte_index]& ~0xFFF);
            }
          }
        }
      }
    }
  }
}

void do_run_elf_thread() {
  Elf32_Ehdr elf;
  thread_t* current = thread_current();
  exec_t* exec = current->data;
  kprintf("load elf %s\n", exec->filename);
  u32 fd = sys_open(exec->filename, 0);
  if (fd < 0) {
    kprintf("open elf %s error\n", exec->filename);
    return;
  }
  u32 nbytes = sys_read(fd, &elf, sizeof(Elf32_Ehdr));
  Elf32_Ehdr* elf_header = (Elf32_Ehdr*)&elf;
  u32* kentry = 0;
  if (elf_header->e_ident[0] == ELFMAG0 || elf_header->e_ident[1] == ELFMAG1) {
    kentry = load_elf(elf_header, fd, current->context.page_dir);
  } else {
    kprintf("load faild not elf\n");
  }
  entry_fn entry = elf_header->e_entry;
  entry(0, exec->argv);
}

u32 sys_exec(char* filename, char* const argv[]) {
  exec_t* data = kmalloc(sizeof(exec_t));
  data->filename = filename;
  data->argv = argv;
  thread_t* t = thread_create((u32*)&do_run_elf_thread, data);

  context_t* context = &t->context;
  u64* page_dir_ptr_tab = mm_alloc_zero_align(sizeof(u64) * 4, 0x1000);
  // init 2GB
  // map_2gb(page_dir_ptr_tab, page_dir, page_tab);
  page_clone(t->context.page_dir, page_dir_ptr_tab);
  context->page_dir = page_dir_ptr_tab;

  thread_run(t);
  return 0;
}