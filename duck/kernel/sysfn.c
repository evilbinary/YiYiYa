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
  thread_t* current = thread_current();
  // kprintf("sys print %d %s\n", current->id, s);
  kprintf("%s", s);
  return 0;
}

void sys_test() {
  thread_t* current = thread_current();
  kprintf("sys test %d\n", current->id);

  kprintf("-------dump thread %d-------------\n", current->id);
  thread_dump(current);
}

void sys_dumps() {
  thread_t* current = thread_current();
  thread_dumps();
}

int sys_print_at(char* s, u32 x, u32 y) {
  kprintf("%s", s);
  return 0;
}

size_t sys_ioctl(u32 fd, u32 cmd, void* args) {
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

  log_debug("sys ioctl fd %d %s cmd %x ret %x\n", fd, f->name, cmd, ret);
  return ret;
}

u32 sys_open(char* name, int attr) {
  // mm_dump();
  // kprintf("open %s attr %x\n",name,attr&O_CREAT==O_CREAT);
  if (name == NULL) {
    kprintf("open name is null\n");
    return -1;
  }
  thread_t* current = thread_current();
  if (current == NULL) {
    kprintf(" cannot find current thread\n");
    return -1;
  }
  int f = thread_find_fd_name(current, name);
  if (f >= 0) {
    fd_t* fd = thread_find_fd_id(current, f);
    fd->offset = 0;
    log_debug("sys open name return : %s fd: %d\n", name, f);
    return f;
  }
  vnode_t* file = vfs_open_attr(NULL, name, attr);
  if (file == NULL) {
    kprintf("sys open file %s error, attr %x \n", name, attr);
    return -1;
  }
  fd_t* fd = fd_new(file, DEVICE_TYPE_FILE, name);
  if (fd == NULL) {
    kprintf(" new fd error\n");
    return -1;
  }
  fd->offset = 0;
  f = thread_add_fd(current, fd);
  if (f < 0) {
    kprintf("sys open %s error\n", name);
    return -1;
  }
  log_debug("sys open new name: %s fd:%d fd->id:%d ptr:%x tid:%d\n", name, f,
            fd->id, fd, current->id);
  return f;
}

int sys_close(u32 fd) {
  thread_t* current = thread_current();
  fd_t* f = thread_find_fd_id(current, fd);
  if (f == NULL) {
    kprintf("close not found fd %d tid %d\n", fd, current->id);
    return 0;
  }
  vnode_t* node = f->data;
  if (node == NULL) {
    kprintf("sys close node is null tid %d \n", current->id);
    return -1;
  }
  // reset offset
  f->offset = 0;
  u32 ret = vclose(node);
  return 0;
}

size_t sys_write(u32 fd, void* buf, size_t nbytes) {
  thread_t* current = thread_current();
  fd_t* f = thread_find_fd_id(current, fd);
  if (f == NULL) {
    kprintf("write not found fd %d tid %d\n", fd, current->id);
    thread_dump_fd(current);
    return 0;
  }
  vnode_t* node = f->data;
  if (node == NULL) {
    kprintf("sys write node is null tid %d \n", current->id);
    return -1;
  }
  // kprintf("sys write %d %s fd:%s\n",current->id,buf,f->name);
  u32 ret = vwrite(node, f->offset, nbytes, buf);
  f->offset += nbytes;
  return ret;
}
size_t sys_read(u32 fd, void* buf, size_t nbytes) {
  thread_t* current = thread_current();
  fd_t* f = thread_find_fd_id(current, fd);
  if (f == NULL) {
    kprintf("read not found fd %d tid %d\n", fd, current->id);
    return 0;
  }
  vnode_t* node = f->data;
  if (node == NULL) {
    kprintf("sys read node is null\n");
    return -1;
  }
  u32 ret = vread(node, f->offset, nbytes, buf);
  f->offset += ret;
  return ret;
}

size_t sys_seek(u32 fd, ulong offset, int whence) {
  fd_t* f = thread_find_fd_id(thread_current(), fd);
  if (f == NULL) {
    kprintf("seek not found fd %d\n", fd);
    return 0;
  }
  // set start offset
  if (whence == 0) {  // seek set
    f->offset = offset;
  } else if (whence == 1) {  // seek current
    f->offset += offset;
  } else if (whence == 2) {  // seek end
    vnode_t* file = f->data;
    if (file != NULL) {
      f->offset = file->length + offset;
    }
  } else {
    kprintf("seek whence error %d\n", whence);
    return -1;
  }
  return f->offset;
}

size_t sys_yeild() { thread_yield(); }

void sys_exit(int status) {
  thread_t* current = thread_current();
  thread_exit(current, status);
  // thread_dumps();
  log_debug("sys exit tid %d %s status %d\n", current->id, current->name,
            status);
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

#define PAGE_CLONE 1

u32 sys_exec(char* filename, char* const argv[], char* const envp[]) {
  int fd = sys_open(filename, 0);
  if (fd < 0) {
    sys_close(fd);
    kprintf("sys exec file not found %s\n", filename);
    return -1;
  }
  thread_t* current = thread_current();
  fd_t* f = thread_find_fd_id(current, fd);
  if (f == NULL) {
    kprintf("read not found fd %d tid %d\n", fd, current->id);
    return 0;
  }
  sys_close(fd);
  u8* vstack3 = STACK_ADDR;
  thread_t* t = thread_create_ex_name(filename, (u32*)&run_elf_thread,
                                      THREAD_STACK_SIZE, NULL, USER_MODE, 0);
  thread_reset_stack3(t, vstack3);
  t->context.kernel_page_dir = current->context.kernel_page_dir;
#ifdef PAGE_CLONE
  t->context.page_dir = page_alloc_clone(current->context.page_dir);
#else
  t->context.page_dir = current->context.page_dir;
#endif

  // init pwd
  vnode_t* node = f->data;
  if (node == NULL) {
    kprintf("sys exec node is null pwd\n");
    return -1;
  }
  if (node->parent != NULL) {
    t->vfs->pwd = node->parent;
  } else {
    t->vfs->pwd = node;
  }

  // init vmm
  t->vmm = vmemory_area_create(HEAP_ADDR, MEMORY_HEAP_SIZE, MEMORY_HEAP);
  vmemory_area_t* vmexec =
      vmemory_area_create(EXEC_ADDR, MEMORY_EXEC_SIZE, MEMORY_EXEC);
  vmemory_area_add(t->vmm, vmexec);
  vmemory_area_t* stack =
      vmemory_area_create(vstack3, THREAD_STACK_SIZE, MEMORY_STACK);
  vmemory_area_add(t->vmm, stack);

  // init data
  int argc = 0;
  while (argv != NULL && argv[argc] != NULL) {
    argc++;
  }

  exec_t* data = kmalloc(sizeof(exec_t));
  data->filename = filename;
  data->argv = argv;
  data->argc = argc;
  data->envp = envp;
  t->exec = data;
  // init fds
  for (int i = 0; i < 3; i++) {
    t->fds[i] = current->fds[i];
  }
  // thread_dump(t);
  thread_run(t);
  return 0;
}

int sys_fork() {
  thread_t* current = thread_current();
  if (current == NULL) {
    kprintf("current is null\n");
    return -1;
  }
  thread_t* copy_thread = thread_clone(current, STACK_ADDR, THREAD_STACK_SIZE);

#ifdef LOG_DEBUG
  log_debug("-------dump current thread %d %s-------------\n", current->id);
  thread_dump(current);
  log_debug("-------dump clone thread %d-------------\n", copy_thread->id);
  thread_dump(copy_thread);
#endif

  interrupt_context_t* context = copy_thread->context.esp0;
  context_ret(context) = 0;

  thread_run(copy_thread);
  return current->id;
}

int sys_pipe(int fds[2]) {
  thread_t* current = thread_current();
  vnode_t* node = pipe_make(PAGE_SIZE);
  fd_t* fd0 = fd_new(node, DEVICE_TYPE_VIRTUAL, "pipe0");
  fd_t* fd1 = fd_new(node, DEVICE_TYPE_VIRTUAL, "pipe1");
  fds[0] = thread_add_fd(current, fd0);
  ;
  fds[1] = thread_add_fd(current, fd1);
  ;
  return 0;
}

int sys_getpid() {
  thread_t* current = thread_current();
  log_debug("sys get pid %d\n", current->id);
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
  kprintf("sys dup %d %s\n", newfd, fd->name);
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

int sys_readdir(int fd, int index, void* dirent) {
  thread_t* current = thread_current();
  fd_t* findfd = thread_find_fd_id(current, fd);
  if (fd == NULL) {
    kprintf("readdir not found fd %d\n", fd);
    return 0;
  }
  u32 ret = vreaddir(findfd->data, dirent, index);
  return ret;
}

int sys_brk(int addr) {
  thread_t* current = thread_current();
  log_debug("sys sbrk tid:%x addr:%x\n", current->id, addr);

  vmemory_area_t* vm = vmemory_area_find_flag(current->vmm, MEMORY_HEAP);
  if (vm == NULL) {
    kprintf("sys brk not found vm\n");
    return 0;
  }
  if (addr == 0) {
    if (vm->vend == vm->vaddr) {
      vm->vend = vm->vaddr + addr;
    }
    addr = vm->vend;
    kprintf("sys sbrk return first addr:%x\n", addr);
    return addr;
  }
  vm->vend = addr;
  log_debug("sys sbrk return addr:%x\n", vm->vend);
  return vm->vend;
}

int sys_readv(int fd, iovec_t* vector, int count) {
  int ret = 0;
  int n;
  int i;
  for (i = 0; i < count; i++, vector++) {
    n = sys_read(fd, vector->iov_base, vector->iov_len);
    if (n < 0) return n;
    ret += n;
    if (n != vector->iov_len) break;
  }
  return ret;
}

int sys_writev(int fd, iovec_t* vector, int count) {
  int ret = 0;
  int n;
  int i;
  for (i = 0; i < count; i++, vector++) {
    n = sys_write(fd, vector->iov_base, vector->iov_len);
    if (n < 0) return n;
    ret += n;
    if (n != vector->iov_len) break;
  }
  return ret;
}

int sys_chdir(const char* path) {
  int ret = 0;
  thread_t* current = thread_current();
  int fd = sys_open(path, 0);
  if (fd < 0) {
    return -1;
  }
  sys_fchdir(fd);
  sys_close(fd);
  return ret;
}

void* sys_mmap2(void* addr, int length, int prot, int flags, int fd,
                int pgoffset) {
  int ret = 0;
  ret = addr;
  if (fd >= 0 || pgoffset > 0) {
    kprintf("mmap2 system call : fd = %d, prot = %x, pgoffset = %d\n", fd, prot,
            pgoffset);
    return -1;
  }

  return ret;
}

int sys_mprotect(const void* start, size_t len, int prot) {
  int ret = 0;
  kprintf("sys mprotect not impl\n");

  return ret;
}

int sys_rt_sigprocmask(int h, void* set, void* old_set) {
  kprintf("sys sigprocmask not impl\n");
  return 0;
}

unsigned int sys_alarm(unsigned int seconds) {
  kprintf("sys alarm not impl\n");
  return -1;
}

int sys_unlink(const char* pathname) {
  kprintf("sys unlink not impl %s\n", pathname);
  return -1;
}

int sys_rename(const char* old, const char* new) {
  kprintf("sys rename not impl %s\n", old);

  return -1;
}

int sys_set_thread_area(void* set) {
  kprintf("sys set thread area not impl \n");
  return 1;
}

int sys_getdents64(unsigned int fd, vdirent_t* dir, unsigned int count) {
  thread_t* current = thread_current();
  fd_t* findfd = thread_find_fd_id(current, fd);
  if (fd == NULL) {
    kprintf("getdents64 not found fd %d\n", fd);
    return 0;
  }
  u32 ret = vreaddir(findfd->data, dir, count);
  return ret;
}

int sys_munmap(void* addr, size_t size) {
  kprintf("sys munmap not impl addr: %x size: %d\n", addr, size);
  return 1;
}

int sys_fcntl64(int fd, int cmd, void* arg) {
  kprintf("sys fcntl64 not impl fd: %d cmd: %x\n", fd, cmd);

  return 1;
}

int sys_getcwd(char* buf, size_t size) {
  thread_t* current = thread_current();
  if (current == NULL) {
    kprintf("current is null\n");
    return -1;
  }
  int ret = 0;
  vfs_t* vfs = current->vfs;
  if (vfs == NULL) {
    return -1;
  }
  if (vfs->pwd != NULL) {
    ret == kstrcpy(buf, vfs->pwd->name);
  } else {
    buf[0] = 0;
  }
  return ret;
}

int sys_fchdir(int fd) {
  u32 ret = 0;
  thread_t* current = thread_current();
  fd_t* f = thread_find_fd_id(current, fd);
  if (f == NULL) {
    kprintf("read not found fd %d tid %d\n", fd, current->id);
    return -1;
  }
  vnode_t* node = f->data;
  if ((node->flags & V_DIRECTORY) == V_DIRECTORY) {
    current->vfs->pwd = node;
  } else {
    kprintf("not directory\n");
    return -1;
  }
  return ret;
}

int sys_clone(void* fn, void* stack, void* arg) {
  thread_t* current = thread_current();
  if (current == NULL) {
    kprintf("current is null\n");
    return -1;
  }
  thread_t* copy_thread = thread_clone(current, STACK_ADDR, THREAD_STACK_SIZE);
#ifdef LOG_DEBUG
  kprintf("-------dump current thread %d %s-------------\n", current->id);
  thread_dump(current);
  kprintf("-------dump clone thread %d-------------\n", copy_thread->id);
  thread_dump(copy_thread);
#endif

  interrupt_context_t* context = copy_thread->context.esp0;
  context_ret(context) = 0;
  context_set_entry(&copy_thread->context, fn);

  thread_run(copy_thread);
  return copy_thread->id;
}

int sys_llseek(int fd, off_t offset_hi, off_t offset_lo, off_t* result,
               int whence) {
  *result = 0;
  return sys_seek(fd, offset_hi << 32 | offset_lo, whence);
}

int sys_umask(int mask) {
  thread_t* current = thread_current();
  if (current == NULL) {
    kprintf("current is null\n");
    return -1;
  }
  // todo

  return mask;
}

int sys_stat(const char* path, struct stat* stat) {
  if (stat == NULL) {
    return -1;
  }
  int fd = sys_open(path, 0);
  return sys_fstat(fd, stat);
}

int sys_fstat(int fd, struct stat* stat) {
  if (stat == NULL) {
    return -1;
  }
  thread_t* current = thread_current();
  fd_t* f = thread_find_fd_id(current, fd);
  if (f == NULL) {
    kprintf("stat not found fd %d tid %d\n", fd, current->id);
    return 0;
  }
  vnode_t* node = f->data;
  u32 cmd = IOC_STAT;
  u32 ret = vioctl(node, cmd, stat);
  return ret;
}

int sys_self(void* t) {
  if (t == NULL) return -2;
  thread_t* current = thread_current();
  if (current == NULL) return -1;
  kmemcpy(t, current, sizeof(thread_t));
  return 1;
}

int sys_clock_nanosleep(int clock, int flag,struct timespec* req,
                        struct timespec* rem) {
  // kprintf("sys_clock_nanosleep %d %d\n",req->tv_sec,req->tv_nsec);
  schedule_sleep(req->tv_sec*1000+req->tv_nsec);
  return 0;
}



void sys_fn_init(void** syscall_table) {
  syscall_table[SYS_READ] = &sys_read;
  syscall_table[SYS_WRITE] = &sys_write;
  syscall_table[SYS_YIELD] = &sys_yeild;
  syscall_table[SYS_PRINT] = &sys_print;
  syscall_table[SYS_PRINT_AT] = &sys_print_at;
  syscall_table[SYS_IOCTL] = &sys_ioctl;
  syscall_table[SYS_OPEN] = &sys_open;
  syscall_table[SYS_CLOSE] = &sys_close;
  syscall_table[SYS_DEV_READ] = &dev_read;
  syscall_table[SYS_DEV_WRITE] = &dev_write;
  syscall_table[SYS_DEV_IOCTL] = &dev_ioctl;
  syscall_table[SYS_EXEC] = &sys_exec;
  syscall_table[SYS_TEST] = &sys_test;
  syscall_table[SYS_EXIT] = &sys_exit;
  syscall_table[SYS_MAP] = &sys_vmap;
  syscall_table[SYS_UMAP] = &sys_vumap;
  syscall_table[SYS_SEEK] = &sys_seek;
  syscall_table[SYS_VALLOC] = &sys_valloc;
  syscall_table[SYS_VFREE] = &sys_vfree;
  syscall_table[SYS_VHEAP] = &sys_vheap;
  syscall_table[SYS_FORK] = &sys_fork;
  syscall_table[SYS_PIPE] = &sys_pipe;
  syscall_table[SYS_GETPID] = &sys_getpid;
  syscall_table[SYS_GETPPID] = &sys_getppid;
  syscall_table[SYS_DUP] = &sys_dup;
  syscall_table[SYS_DUP2] = &sys_dup2;
  syscall_table[SYS_READDIR] = &sys_readdir;
  syscall_table[SYS_BRK] = &sys_brk;
  syscall_table[SYS_READV] = &sys_readv;
  syscall_table[SYS_WRITEV] = &sys_writev;
  syscall_table[SYS_CHDIR] = &sys_chdir;
  syscall_table[SYS_MMAP2] = &sys_mmap2;
  syscall_table[SYS_MPROTECT] = &sys_mprotect;
  syscall_table[SYS_RT_SIGPROCMASK] = &sys_rt_sigprocmask;

  syscall_table[SYS_ALARM] = &sys_alarm;
  syscall_table[SYS_UNLINK] = &sys_unlink;
  syscall_table[SYS_RENAME] = &sys_rename;

  syscall_table[SYS_RENAME] = &sys_rename;

  syscall_table[SYS_SET_THREAD_AREA] = &sys_set_thread_area;
  syscall_table[SYS_DUMPS] = &sys_dumps;

  syscall_table[SYS_GETDENTS64] = &sys_getdents64;
  syscall_table[SYS_MUNMAP] = &sys_munmap;

  syscall_table[SYS_FCNT64] = &sys_fcntl64;
  syscall_table[SYS_GETCWD] = &sys_getcwd;
  syscall_table[SYS_CHDIR] = &sys_chdir;
  syscall_table[SYS_FCHDIR] = &sys_fchdir;
  syscall_table[SYS_CLONE] = &sys_clone;
  syscall_table[SYS_LLSEEK] = &sys_llseek;

  syscall_table[SYS_UMASK] = &sys_umask;

  syscall_table[SYS_STAT] = &sys_stat;
  syscall_table[SYS_FSTAT] = &sys_fstat;
  syscall_table[SYS_SELF] = &sys_self;

  syscall_table[SYS_CLOCK_NANOSLEEP] = &sys_clock_nanosleep;
}