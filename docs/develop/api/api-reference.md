# API 参考

## 系统调用

### 进程管理

#### 进程创建
```c
pid_t fork(void);
pid_t vfork(void);
int execve(const char *pathname, char *const argv[], char *const envp[]);
```

#### 进程控制
```c
int kill(pid_t pid, int sig);
pid_t getpid(void);
pid_t getppid(void);
int wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);
```

### 内存管理

#### 内存分配
```c
void *malloc(size_t size);
void free(void *ptr);
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
```

### 文件系统

#### 文件操作
```c
int open(const char *pathname, int flags, mode_t mode);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
```

#### 目录操作
```c
int mkdir(const char *pathname, mode_t mode);
int rmdir(const char *pathname);
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dir);
int closedir(DIR *dir);
```

### 设备操作

#### 设备访问
```c
int device_open(const char *name);
int device_ioctl(int fd, u32 cmd, void *args);
```

### 网络接口

#### Socket 操作
```c
int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```

## 内核 API

### 线程管理

#### 线程创建
```c
thread_t* thread_create(void (*func)(void*), void* arg);
thread_t* thread_create_name(const char* name, void (*func)(void*), void* arg);
thread_t* thread_create_name_level(const char* name, void (*func)(void*), 
                                    void* arg, u32 level);
int thread_run(thread_t* thread);
void thread_exit(void);
```

#### 线程调度
```c
void schedule(void);
void schedule_sleep(u64 ns);
void thread_yield(void);
```

### 内存管理

#### 内核内存
```c
void* kmalloc(size_t size, u32 type);
void kfree(void* ptr);
void* page_alloc(u32 count);
void page_free(void* addr, u32 count);
```

#### 内存映射
```c
int page_map(u32 vaddr, u32 paddr, u32 flags);
void page_unmap(u32 vaddr);
```

### 设备驱动

#### 设备注册
```c
int device_add(device_t* dev);
device_t* device_find(u32 id);
device_t* device_find_by_name(const char* name);
```

#### 设备操作
```c
size_t device_read(device_t* dev, void* buf, size_t len);
size_t device_write(device_t* dev, const void* buf, size_t len);
int device_ioctl(device_t* dev, u32 cmd, void* args);
```

### 中断处理

#### 中断注册
```c
int interrupt_register(u32 no, interrupt_handler_t handler, void* data);
int interrupt_unregister(u32 no);
void interrupt_enable(u32 no);
void interrupt_disable(u32 no);
```

### 时间管理

#### 定时器
```c
u64 timer_get_ticks(void);
u64 timer_get_ms(void);
u64 timer_get_us(void);
void timer_delay(u64 ms);
```

### 同步原语

#### 互斥锁
```c
void mutex_init(mutex_t* mutex);
void mutex_lock(mutex_t* mutex);
void mutex_unlock(mutex_t* mutex);
```

#### 信号量
```c
void sem_init(sem_t* sem, u32 value);
void sem_wait(sem_t* sem);
void sem_post(sem_t* sem);
```

### 日志系统

#### 日志输出
```c
void log_debug(const char* fmt, ...);
void log_info(const char* fmt, ...);
void log_warn(const char* fmt, ...);
void log_error(const char* fmt, ...);
```

## 驱动开发 API

### 网络驱动

#### 网络设备操作
```c
typedef struct net_device {
    u8 mac_addr[6];
    u32 mtu;
    int (*send)(struct net_device* dev, const void* buf, size_t len);
    int (*recv)(struct net_device* dev, void* buf, size_t len);
} net_device_t;
```

### 存储驱动

#### 块设备操作
```c
typedef struct block_device {
    u32 block_size;
    u64 total_blocks;
    int (*read)(struct block_device* dev, u64 block, void* buf, u32 count);
    int (*write)(struct block_device* dev, u64 block, const void* buf, u32 count);
} block_device_t;
```

### 显示驱动

#### 帧缓冲操作
```c
typedef struct framebuffer {
    u32* buffer;
    u32 width;
    u32 height;
    u32 bpp;
    void (*flush)(struct framebuffer* fb, u32 x, u32 y, u32 w, u32 h);
} framebuffer_t;
```

## 标准库

YiYiYa 支持 musl libc，提供标准的 POSIX 接口。

### 字符串操作
```c
size_t strlen(const char *s);
char *strcpy(char *dest, const char *src);
char *strcat(char *dest, const char *src);
int strcmp(const char *s1, const char *s2);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
```

### 数学运算
```c
double sin(double x);
double cos(double x);
double sqrt(double x);
```

## 参考资料

- [系统调用详细列表](../material/syscall.md)
- [内核开发指南](../develop/内核开发.md)
- [驱动开发指南](../develop/驱动开发.md)
