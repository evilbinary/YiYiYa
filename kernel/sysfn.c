/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/

#include "sysfn.h"

#include "fd.h"
#include "kernel.h"
#include "kernel/elf.h"
#include "loader.h"
#include "thread.h"
#include "vfs.h"

int sys_print(char* s) {
  kprintf("%s", s);
  return 0;
}

void sys_test() {
   thread_t* current = thread_current();
   kprintf("sys test %d\n",current->id);
}

int sys_print_at(char* s, u32 x, u32 y) {
  set_cursor(x, y);
  kprintf("%s", s);
  return 0;
}

// size_t sys_ioctl(u32 fd, u32 cmd, ...) {
//   u32 ret = 0;
//   fd_t* f = thread_find_fd_id(thread_current(), fd);
//   if (f == NULL) {
//     kprintf("ioctl not found fd %d\n", fd);
//     return 0;
//   }
//   va_list args;
//   va_start(args, cmd);
//   vnode_t* node = f->data;
//   ret = vioctl(node, cmd, args);
//   va_end(args);
//   return ret;
// }

size_t sys_ioctl(u32 fd, u32 cmd, va_list args) {
  u32 ret = 0;
  fd_t* f = thread_find_fd_id(thread_current(), fd);
  if (f == NULL) {
    // f = find_fd(fd);
  }
  if (f == NULL) {
    kprintf("ioctl not found fd %d\n", fd);
    return 0;
  }
  vnode_t* node = f->data;
  ret = vioctl(node, cmd, args);

#ifdef SYS_DEBUG
  kprintf("sys ioctl fd %d %s cmd %x ret %x\n", fd, f->name, cmd, ret);
#endif
  return ret;
}

u32 sys_open(char* name, int attr) {
  // mm_dump();
  // kprintf("open %s attr %x\n",name,attr&O_CREAT==O_CREAT);

  thread_t* current = thread_current();
  if (current == NULL) {
    kprintf(" cannot find current thread\n");
    return -1;
  }
  int f = thread_find_fd_name(current, name);
  if (f >= 0) {
    kprintf("sys open name return : %s fd: %d\n", name, f);
    return f;
  }
  vnode_t* file = vfs_open(NULL, name,attr);  
  if(file==NULL){
    kprintf("sys open file %s error\n", name);
    return -1;
  }
  fd_t* fd = fd_new(file, DEVICE_TYPE_FILE, name);
  if (fd == NULL) {
    kprintf(" new fd error\n");
    return -1;
  }

  f = thread_add_fd(current, fd);
  if (f < 0) {
    kprintf("sys open %s error\n", name);
    return -1;
  }
  kprintf("sys open new name: %s fd:%d id:%d ptr:%x\n", name, f,fd->id,fd);
  return f;
}

void sys_close(u32 fd) {}

size_t sys_write(u32 fd, void* buf, size_t nbytes) {
  thread_t* current=thread_current();
  fd_t* f = thread_find_fd_id(current, fd);
  if (f == NULL) {
    kprintf("write not found fd %d tid %d\n", fd,current->id);
    thread_dump_fd(current);
    return 0;
  }
  vnode_t* node = f->data;
  if (node == NULL) {
    kprintf("sys write node is null tid %d \n",current->id);
    return -1;
  }
  //kprintf("sys write %d %s fd:%s\n",current->id,buf,f->name);
  u32 ret = vwrite(node, f->offset, nbytes, buf);
  return ret;
}
size_t sys_read(u32 fd, void* buf, size_t nbytes) {
  thread_t* current=thread_current();
  fd_t* f = thread_find_fd_id(current, fd);
  if (f == NULL) {
    kprintf("read not found fd %d tid %d\n", fd,current->id);
    return 0;
  }
  vnode_t* node = f->data;
  if (node == NULL) {
    kprintf("sys read node is null\n");
    return -1;
  }
  u32 ret = vread(node, f->offset, nbytes, buf);
  return ret;
}

size_t sys_seek(u32 fd, ulong offset) {
  fd_t* f = thread_find_fd_id(thread_current(), fd);
  if (f == NULL) {
    kprintf("seek not found fd %d\n", fd);
    return 0;
  }
  f->offset = offset;
  return 1;
}

size_t sys_yeild() { thread_yield(); }

void sys_exit(int status) {
  thread_t* current = thread_current();
  thread_exit(current);
  thread_dumps();
  kprintf("sys exit %d\n", status);
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

u32 sys_exec(char* filename, char* const argv[],char *const envp[]) {
  int fd=sys_open(filename,0);
  if(fd<0){
    sys_close(fd);
    kprintf("sys exec file not found %s\n",filename);
    return -1;
  }
  sys_close(fd);

  thread_t* current = thread_current();
  u8* stack0 = kmalloc(THREAD_STACK_SIZE);
  thread_t* t = thread_create_ex((u32*)&run_elf_thread, stack0, STACK_ADDR,
                                 THREAD_STACK_SIZE, NULL);
  context_t* context = &t->context;
  // init 2GB
  // map_2gb(page_dir_ptr_tab, PAGE_P | PAGE_USU | PAGE_RWW);
  context->page_dir = page_alloc_clone(t->context.kernel_page_dir);

  // init vmm
  t->vmm = vmemory_area_create(HEAP_ADDR, MEMORY_CREATE_SIZE, MEMORY_HEAP);
  vmemory_area_t* vmexec =
      vmemory_area_create(EXEC_ADDR, MEMORY_EXEC_SIZE, MEMORY_EXEC);
  vmemory_area_add(t->vmm, vmexec);
  vmemory_area_t* vmdata =
      vmemory_area_create(DATA_ADDR, PAGE_SIZE, MEMORY_DATA);
  vmemory_area_add(t->vmm, vmdata);
  vmemory_area_t* stack =
      vmemory_area_create(DATA_ADDR, PAGE_SIZE, MEMORY_STACK);
  vmemory_area_add(t->vmm, stack);

  // init stack
  // void* stackaddr = kmalloc_alignment(sizeof(exec_t), PAGE_SIZE);
  // map_page_on(t->context.page_dir, stack->vaddr, stackaddr,PAGE_P | PAGE_USU
  // | PAGE_RWW);

  // init data
  void* paddr = kmalloc_alignment(sizeof(exec_t), PAGE_SIZE);
  map_page_on(t->context.page_dir, vmdata->vaddr, paddr,
              PAGE_P | PAGE_USU | PAGE_RWW);
  exec_t* data = paddr;
  kstrcpy(data->filename, filename);
  data->argv = argv;
  t->data = vmdata->vaddr;

  //init fds
  for(int i=0;i<3;i++){
    t->fds[i]=current->fds[i];
  }
  //thread_dump_fd(t);
  thread_run(t);
  return 0;
}

int sys_fork() {
  thread_t* current = thread_current();
  thread_t* copy_thread = thread_clone(current, STACK_ADDR, THREAD_STACK_SIZE);
  copy_thread->pid = current->id;


#ifdef ARM
  kprintf("c eax no zero\n");
#elif defined(X86)
  interrupt_context_t* c0 = current->context.esp0;
  interrupt_context_t* c = copy_thread->context.esp0;
  kprintf("# current tid %d eip:%x esp:%x ebp:%x \n", current->id, c0->eip,
          c0->esp, c0->ebp);
  kprintf("# new     tid %d eip:%x esp:%x ebp:%x \n", copy_thread->id, c->eip,
          c->esp, c->ebp);
  c->eax = 0;
#endif

  thread_run(copy_thread);
  return copy_thread->id;
}

int sys_pipe(int fds[2]) {
  thread_t* current = thread_current();
  vnode_t* node = pipe_make(PAGE_SIZE);
  fd_t* fd0 = fd_new(node, DEVICE_TYPE_VIRTUAL, "pipe0");
  fd_t* fd1 = fd_new(node, DEVICE_TYPE_VIRTUAL, "pipe1");
  fds[0]=thread_add_fd(current,fd0);;
  fds[1]=thread_add_fd(current,fd1);;
  return 0;
}

int sys_getpid() {
  thread_t* current = thread_current();
#ifdef SYS_DEBUG
  kprintf("sys get pid %d\n",current->id);
#endif
  return current->id;
}

int sys_getppid() {
  thread_t* current = thread_current();
  return current->pid;
}

int sys_dup(int oldfd) {
  thread_t* current = thread_current();
  fd_t* fd = thread_find_fd_id(current, oldfd);
  if (fd == NULL) {
    kprintf("dup not found fd %d\n", oldfd);
    return 0;
  }
  int newfd = thread_add_fd(current, fd);
#ifdef DEBUG_SYS_FN
  kprintf("sys dup %d %s\n",newfd,fd->name);
#endif
  return newfd;
}

int sys_dup2(int oldfd, int newfd) {
  thread_t* current = thread_current();
  fd_t* fd = thread_find_fd_id(current, oldfd);
  if (fd == NULL) {
    kprintf("dup not found fd %d\n", fd);
    return 0;
  }
  fd_t* nfd = thread_find_fd_id(current, newfd);
  if (nfd == NULL) {
    kprintf("dup not found nfd %d\n", nfd);
    return 0;
  }
  fd_close(fd);
  thread_set_fd(current, newfd, fd);
  return newfd;
}

int sys_readdir(int fd,int index,void* dirent){
  thread_t* current = thread_current();
  fd_t* findfd = thread_find_fd_id(current, fd);
  if (fd == NULL) {
    kprintf("readdir not found fd %d\n", fd);
    return 0;
  }
  vdirent_t* d= vreaddir(findfd->data,index);
  if(d==NULL){
    return 0;
  }
  kmemmove(dirent,d,sizeof(vdirent_t));
  kfree(d);
  return 1;
}