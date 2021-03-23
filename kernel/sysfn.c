/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "sysfn.h"

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
    kprintf("ioctl not found fd %d\n", fd);
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
  char* last = kstrrstr(name, node->name);
  if (last != NULL) {
    last += kstrlen(node->name);
    if (last[0] == '/') last++;
  }
  vnode_t* file = vfind(node, last);
  if (file == NULL) {
    kprintf("find %s failed\n", last);
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
  open_list[open_number].name = name;
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

size_t sys_seek(u32 fd, ulong offset) {
  fd_t* f = find_fd(fd);
  if (f == NULL) {
    kprintf("read not found fd %d\n", fd);
    return 0;
  }
  f->offset = offset;
  return 1;
}

size_t sys_yeild() { thread_yield(); }

void sys_exit(int status) {
  thread_t* current = thread_current();
  current->state = THREAD_STOPPED;
  thread_remove(current);
}

void* sys_vmap(void* addr, size_t size) {
  thread_t* current = thread_current();
  vmemory_area_t* area = vmemory_area_create(
      current->vmm->vaddr + current->vmm->size, size, MEMORY_HEAP);
  vmemory_area_add(current->vmm, area);
  return area->vaddr;
}

void sys_vumap(void* ptr, size_t size) {
  thread_t* current = thread_current();
  vmemory_area_t* area = vmemory_area_find(current->vmm, ptr, size);
  if (area == NULL) return;
  vmemory_area_free(area);
}

void* sys_valloc(void* addr, size_t size) { return valloc(addr, size); }

void* sys_vheap() {
  thread_t* current = thread_current();
  return current->vmm->vaddr;
}

void sys_vfree(void* addr) {
  // todo
  vfree(addr);
}

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
    // kprintf("type:%d\n\r", phdr[i].p_type);
    switch (phdr[i].p_type) {
      case PT_NULL:
        // kprintf(" %s %x %x %x %s %x %x \r\n", "NULL", phdr[i].p_offset,
        //        phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
        //        phdr[i].p_memsz);
        break;
      case PT_LOAD: {
        // printf(" %s %x %x %x %s %x %x \r\n", "LOAD", phdr[i].p_offset,
        //        phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
        //        phdr[i].p_memsz);
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
      // kprintf("load start:%x vaddr:%x size:%x \n\r", start, vaddr,
      //        phdr[i].p_filesz);
      syscall2(SYS_SEEK, fd, start);
      u32 ret = syscall3(SYS_READ, fd, vaddr, shdr[i].sh_size);
      // map_alignment(page,vaddr,buf,shdr[i].sh_size);
    }
  }
  return entry;
}

void do_run_elf_thread() {
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
  if (elf_header->e_ident[0] == ELFMAG0 || elf_header->e_ident[1] == ELFMAG1) {
    kentry = load_elf(elf_header, fd, current->context.page_dir);
  } else {
    kprintf("load faild not elf\n");
  }
  // map_page_on(current->context.page_dir,current->context.esp,current->context.esp,PAGE_P
  // | PAGE_USU | PAGE_RWW);
  entry_fn entry = elf_header->e_entry;
  u32 ret = entry(0, exec->argv);
  syscall1(SYS_EXIT, ret);
}

u32 sys_exec(char* filename, char* const argv[]) {
  thread_t* t = thread_create((u32*)&do_run_elf_thread, NULL);
  context_t* context = &t->context;
  u64* page_dir_ptr_tab = kmalloc_alignment(sizeof(u64) * 4, 0x1000);

  page_clone_user(t->context.kernel_page_dir, page_dir_ptr_tab);
  // init 2GB
  // map_2gb(page_dir_ptr_tab, PAGE_P | PAGE_USU | PAGE_RWW);
  context->page_dir = page_dir_ptr_tab;

  // init vmm
  t->vmm = vmemory_area_create(HEAP_ADDR, MEMORY_CREATE_SIZE, MEMORY_HEAP);
  vmemory_area_t* vmexec =
      vmemory_area_create(EXEC_ADDR, MEMORY_EXEC_SIZE, MEMORY_EXEC);
  vmemory_area_add(t->vmm, vmexec);
  vmemory_area_t* vmdata =
      vmemory_area_create(DATA_ADDR, PAGE_SIZE, MEMORY_DATA);
  vmemory_area_add(t->vmm, vmdata);
  vmemory_area_t* stack =
      vmemory_area_create(DATA_ADDR, PAGE_SIZE, MEMORY_DATA);
  vmemory_area_add(t->vmm, stack);

  // init data
  void* paddr = kmalloc_alignment(sizeof(exec_t), PAGE_SIZE);
  map_page_on(t->context.page_dir, vmdata->vaddr, paddr,
              PAGE_P | PAGE_USU | PAGE_RWW);
  exec_t* data = paddr;
  kstrcpy(data->filename, filename);
  data->argv = argv;
  t->data = vmdata->vaddr;

  //init stack
  kfree(t->stack3);
  t->stack3 = stack->vaddr;

  thread_run(t);
  return 0;
}