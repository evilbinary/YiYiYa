/*******************************************************************
 * Copyright 2021-present evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "init.h"
#include "gpio.h"

extern boot_info_t* boot_info;
static boot_info_t boot_data;
int cpu_id = 0;

/* 【地址必须是 64 位】原来参数写成 uint（32 位）：Pi5 的外设地址是 40 位
 * （UART0 = 0x1C00030000），传进来就被截断成 0x00030000 —— 每个字节都写进了
 * 低端 RAM，串口自然"一个字都不出"（raspi3 的 0x3F201000 能塞进 32 位，
 * 所以老平台一直没暴露）。参数类型统一按地址宽度给。 */
static void io_write32(u64 port, u32 data) { *(volatile u32*)port = data; }

static u32 io_read32(u64 port) {
  u32 data;
  data = *(volatile u32*)port;
  return data;
}

#ifdef SINGLE_KERNEL
// Linker script symbols - these are absolute addresses, use & to get the address value
extern unsigned long __bss_start;
extern unsigned long __bss_end;
extern unsigned long __start;
extern unsigned long __end;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sidata;

static void print_hex(unsigned long val) {
  const char *hex = "0123456789abcdef";
  for (int i = 60; i >= 0; i -= 4) {
    uart_send_ch(hex[(val >> i) & 0xf]);
  }
}

void init_data() {
  // Copy .data section from FLASH/ROM (load address) to RAM (virtual address)
  // For QEMU raspi3, kernel is loaded directly to RAM, so this is typically a no-op
  char *src = (char *)&_sidata;
  char *dst = (char *)&_sdata;
  char *end = (char *)&_edata;

  uart_send_ch('D');
  uart_send_ch('i');
  uart_send_ch('n');
  uart_send_ch('i');
  uart_send_ch('t');
  uart_send_ch(':');
  uart_send_ch(' ');
  print_hex((unsigned long)src);
  uart_send_ch(' ');
  print_hex((unsigned long)dst);
  uart_send_ch(' ');
  print_hex((unsigned long)end);
  uart_send_ch('\n');
  uart_send_ch('\r');

  // Skip copy if source equals destination (kernel already in RAM)
  if (src != dst) {
    while (dst < end) {
      *dst++ = *src++;
    }
  }
}

void init_segment() {
  int num = boot_data.segments_number++;
  boot_data.segments[num].start = (unsigned long)&__start;
  boot_data.segments[num].size = (unsigned long)&__end - (unsigned long)&__start;
  boot_data.segments[num].type = 1;
  boot_info->kernel_base = (unsigned long)&__start;
  boot_info->kernel_size = (unsigned long)&__end - (unsigned long)&__start;
}

void init_bss() {
  // Clear BSS section to zero
  unsigned long *dst;
  for (dst = (unsigned long *)&__bss_start; dst < (unsigned long *)&__bss_end; dst++) {
    *dst = 0;
  }
}
#else
void* memset(void* s, int c, size_t n) {
  int i;
  for (i = 0; i < n; i++) ((char*)s)[i] = c;
  return s;
}
#endif

/* Weak memset for boot code - overridden by libkernelcommon if linked */
__attribute__((weak)) void* memset(void* s, int c, size_t n) {
  int i;
  for (i = 0; i < n; i++) ((char*)s)[i] = c;
  return s;
}

#ifdef RASPI5
/* ================= 启动信标（诊断用，串口打通后置 0 删除） =================
 * 串口至今打不出一个字，"代码到底有没有跑起来"需要一个独立通道。用之前
 * 【真机实测验证过】的两件事：
 *   1) PSCI SYSTEM_RESET（smc #0, x0=0x84000009）能复位整机；
 *   2) RAM 标记跨热复位保留（platform/raspi5/init.c 探针注释里的结论）。
 * 于是：本次启动是第 N 次 → 前 3 次都立刻复位整机，第 4 次才继续正常启动。
 * 判读（看固件日志）：
 *   · 日志"重来"了 3 次，然后第 4 次继续往下 → 我们的代码确实在 0x80000 跑 ✓
 *   · 一次都不重来 → 代码没跑到这里（或 PSCI 不可用）✗
 * 标记写两处（互备，防被固件启动过程踩掉后无限复位）。 */
/* 【已确认：代码在跑 ⇒ 关掉信标】真机结果："一直在重启" —— 复位本身成功，
 * 说明 boot_beacon() 确实被执行 ✓；循环的原因是本机复位后 RAM 标记不保留，
 * 计数永远是 1 ⇒ 每次都复位。目的已达到（证明代码跑到 init_boot ✓），
 * 继续开着只会无限重启，所以置 0。 */
#define YI_BOOT_BEACON 0
#define YI_BEACON_MAGIC 0x59494245ULL /* 'YIBE' */

static void boot_beacon(void) {
#if YI_BOOT_BEACON
  volatile u64* s0 = (volatile u64*)(uintptr_t)0x20000000ULL;
  volatile u64* s1 = (volatile u64*)(uintptr_t)0x21000000ULL;
  if (s0[0] != YI_BEACON_MAGIC) {
    s0[0] = YI_BEACON_MAGIC;
    s0[1] = 0;
  }
  if (s1[0] != YI_BEACON_MAGIC) {
    s1[0] = YI_BEACON_MAGIC;
    s1[1] = 0;
  }
  s0[1]++;
  s1[1]++;
  {
    u64 n = s0[1] > s1[1] ? s0[1] : s1[1]; /* 取较大的那个（互备） */
    if (n <= 3) {
      register u64 x0 __asm__("x0") = 0x84000009ULL; /* PSCI SYSTEM_RESET */
      __asm__ volatile("smc #0" : "+r"(x0) : : "memory");
      /* SMC 万一没生效就继续往下，不要卡死 */
    }
  }
#endif
}

/* 【Pi5 控制台候选 + 开机自检】RP1 是 PCIe 设备：CPU 侧有预取(0x1C)与非预取
 * (0x1F)两个窗口，而 RP1 内部外设空间又从 0x40000000 起 —— 组合出这些候选：
 *   A 0x1C00030000   预取窗口 + UART 偏移（gpio.h 引固件日志的说法）
 *   B 0x1F00030000   非预取窗口 + UART 偏移（DTB ranges 推出来的说法）
 *   C 0x1C40030000   预取窗口 + RP1 内部 0x40000000 + UART 偏移
 *   D 0x1F40030000   非预取窗口 + 同上
 *   E 0x7D001000     SoC 侧调试口 UART10
 * 资料互相矛盾、之前那版探针又写的是 32 位截断地址（0x1C030000 其实是 DRAM），
 * 所以"串口无反应"的结论都不可信。这里改成：
 *   1) 开机先给每个候选连发 3 个自己的字母（AAA/BBB/...）—— 哪个口是活的，
 *      串口上就先出现哪 3 个字母，一次上电即可定位（【只写不读】：写是
 *      posted 不会中止；读没有设备应答的 PCIe 地址才会拿 UR 打死 CPU）；
 *   2) 之后正常输出广播到全部候选。
 * 没有流控（不读 FR），FIFO 满时字符会被丢；boot 阶段输出短、间隔大够用。 */
static const u64 pi5_console_bases[] = {
    /* A：SoC UART10（serial10）—— 真机 DTB 权威答案：
     *   /chosen stdout-path = serial10，/soc@107c000000/serial@7d001000
     *   ⇒ CPU 侧 = 0x107C000000 + 0x7D001000 = 0x107D001000
     * 固件自己就是在这条口上打印的（用户 USB-TTL 也在这条上）。
     * 旧代码写的 0x7D001000 漏了父节点前缀 ⇒ 写进 DRAM，这是"一个字都不出"的根因。 */
    0x107D001000ULL,
    /* B/C：RP1 UART0 两个窗口（GPIO14/15，固件未选它当控制台；保留作兜底，
     * 只写不读，posted 写不会中止）。 */
    0x1F00030000ULL,
    0x1C00030000ULL,
};
#define PI5_CONSOLE_N \
  (sizeof(pi5_console_bases) / sizeof(pi5_console_bases[0]))

/* 开机自检：每个候选连发 3 个自己的字母（A/B/C/D/E）。 */
void pi5_console_selfid(void) {
  for (unsigned i = 0; i < PI5_CONSOLE_N; i++) {
    unsigned ch = (unsigned)('A' + i);
    for (int k = 0; k < 3; k++) {
      *(volatile unsigned int*)(uintptr_t)pi5_console_bases[i] = ch;
      asm volatile("dsb sy" ::: "memory");
    }
  }
}
#endif

// Raspberry Pi 3/5 UART (PL011)
void uart_init() {
#ifdef RASPI5
  /* 【Pi5：绝不能碰 legacy GPIO 寄存器 —— 这是"BL31 之后一个字都没有"的根因】
   * Pi5 的外设都在 RP1 南桥里，CPU 侧窗口 0x1C00000000：
   *   UART0 = 窗口 +0x30000 ✓ 有效（固件自己就用它打印）；
   *   而 GPFSEL1 / GPIO_PUP_PDN_CNTRL_REG0 这些 BCM2711 风格寄存器在 Pi5 上
   *   【不存在】（见 duck/platform/raspi5/gpio.h 的说明），算出来的地址是
   *   窗口 +0x200004 / +0x2000E4 —— PCIe 上没有设备应答，读回来是 UR →
   *   同步外部中止。而这里 MMU 还没开、异常向量也没建 ⇒ CPU 直接静默卡死。
   * 引脚复用/上下拉/波特率分频全部交给固件：config.txt 的 enable_uart=1
   * （+ uart0-pi5 overlay）已经配好，BL31 刚才就是用这条串口打印的。
   * 这里只把 UART 自身的 LCRH/CR 重新置一次（幂等：不动 IBRD/FBRD ⇒
   * 波特率还是固件那个；万一交接前 TX 被关了也能拉起来）。 */
  for (unsigned i = 0; i < PI5_CONSOLE_N; i++) {
    volatile unsigned int* u =
        (volatile unsigned int*)(uintptr_t)pi5_console_bases[i];
    u[0x2C / 4] = (0b11 << 5) | (1 << 4);  // LCRH: 8n1 + FIFO
    u[0x30 / 4] = 0x301;                   // CR: UARTEN|TXE|RXE
  }
  return;
#else
  register unsigned int r;

  /* initialize UART */
  *UART0_CR = 0;  // turn off UART0

  /* map UART0 to GPIO pins */
  r = *GPFSEL1;
  r &= ~((7 << 12) | (7 << 15));  // gpio14, gpio15
  r |= (4 << 12) | (4 << 15);     // alt0
  *GPFSEL1 = r;

  *GPPUD = 0;  // enable pins 14 and 15
  r = 150;
  while (r--) {
    asm volatile("nop");
  }
  *GPPUDCLK0 = (1 << 14) | (1 << 15);
  r = 150;
  while (r--) {
    asm volatile("nop");
  }
  *GPPUDCLK0 = 0;  // flush GPIO setup

  *UART0_ICR = 0x7FF;  // clear interrupts
  *UART0_IBRD = 2;     // 115200 baud
  *UART0_FBRD = 0xB;
  /* 8n1 + FIFO 使能(FEN=bit4)：理由同上（QEMU PL011 无 FEN 时不接收 RX） */
  *UART0_LCRH = (0b11 << 5) | (1 << 4);  // 8n1 + FIFO enable
  *UART0_CR = 0x301;        // enable Uart, Tx, Rx
#endif  /* !RASPI5 */
}

void uart_send_ch(unsigned int c) {
#ifdef RASPI5
  /* 【候选 A 已确认为活控制台（实测串口先打出 AAA）】对它做有界 FR 等待：
   * 不做的话 16 字节 FIFO 会被连续写入冲爆 —— 实测"前 ~30 字符正常、
   * 之后乱码"（Dinit: ...000f7000 00000000tbnretyd）。
   * 候选 A 是真实设备（serial10），读它的 FR 是安全的。 */
  for (int t = 0; t < 200000; t++) {
    if ((io_read32(pi5_console_bases[0] + 0x18) & 0x20) == 0) {
      break;
    }
  }
  *(volatile unsigned int*)(uintptr_t)pi5_console_bases[0] = c;
  asm volatile("dsb sy" ::: "memory");
  /* 兜底候选（RP1 两个窗口）：【只写不读】—— 读没有设备应答的 PCIe 地址
   * 会拿 UR 把 CPU 打死；写是 posted，最坏只是丢字节。 */
  for (unsigned i = 1; i < PI5_CONSOLE_N; i++) {
    *(volatile unsigned int*)(uintptr_t)pi5_console_bases[i] = c;
  }
  return;
#else
  /* 【有界等待】UART 没有波特率时钟时字节排不出去，FIFO 满后对 DR 的写会
   * 永远阻塞、把 CPU 卡死（实测教训，见 raspi5/init.c 的 uart_putc 注释）。
   * 最多等 ~20 万次就放弃这个字节，绝不拿整机赌一个字符。 */
  for (int t = 0; t < 200000; t++) {
    if ((io_read32(UART0_FR) & 0x20) == 0) {
      break;
    }
  }
  io_write32(UART0_DR, c);
#endif
}

char uart_get_ch() {
  unsigned int c;
  while (io_read32(UART0_FR) & 0x10) {
  }
  c = io_read32(UART0_DR);
  return c;
}

static void print_string(const unsigned char* str) {
  while (*str) {
    uart_send_ch(*str);
    ++str;
  }
}

void itoa(char* buf, int base, long d) {
  char* p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;

  if (base == 'd' && d < 0) {
    *p++ = '-';
    buf++;
    ud = -d;
  } else if (base == 'x')
    divisor = 16;

  do {
    int remainder = ud % divisor;
    *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
  } while (ud /= divisor);

  *p = 0;

  p1 = buf;
  p2 = p - 1;
  while (p1 < p2) {
    char tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
    p1++;
    p2--;
  }
}

void printf(const char* format, ...) {
  char** arg = (char**)&format;
  int c;
  char buf[20];

  arg++;

  while ((c = *format++) != 0) {
    if (c != '%')
      uart_send_ch(c);
    else {
      char* p;

      c = *format++;
      switch (c) {
        case 'd':
        case 'u':
        case 'x':
          itoa(buf, c, *((long*)arg++));
          p = buf;
          goto string;
          break;

        case 's':
          p = *arg++;
          if (!p) p = "(null)";

        string:
          while (*p) uart_send_ch(*p++);
          break;

        default:
          uart_send_ch(*((int*)arg++));
          break;
      }
    }
  }
}

extern void* _stack_svc;

void init_boot_info() {
  boot_info = &boot_data;
  boot_info->version = BOOT_VERSION;
  boot_info->kernel_origin_base = KERNEL_ORIGIN_BASE;
  boot_info->kernel_base = KERNEL_BASE;
  /* 内核实际大小直接由链接符号计算：config.h 的 KERNEL_SIZE 是构建期生成
   * 的陈旧值（生成挂钩经常不触发）， 曾导致 heap 起点切进 .bss/.stack，
   * 线程创建时把内核内存踩坏 */
  {
    extern unsigned long __start;
    extern unsigned long __end;
    boot_info->kernel_size = (unsigned long)&__end - (unsigned long)&__start;
  }
  boot_info->tss_number = MAX_CPU;
  boot_info->second_boot_entry = SECOND_BOOT_ENTRY;
  boot_info->segments_number = 0;
  boot_info->kernel_stack = _stack_svc;
}

void init_disk() {
  boot_info->disk.hpc = 2;
  boot_info->disk.spt = 18;
  boot_info->disk.type = 1;
}

void init_display() {
  boot_info->disply.mode = 1;
  boot_info->disply.video = 0xB8000;
  boot_info->disply.height = 25;
  boot_info->disply.width = 80;
}

void init_memory() {
  int count = 0;
  memory_info_t* ptr = boot_info->memory;
  boot_info->total_memory = 0;

#ifdef RASPI5
  // Raspberry Pi 5 (BCM2712): DRAM 从 0 开始，本机 1GB → [0, 0x40000000)。
  // 上界必须同时满足两条，否则都会踩到"把非 RAM 当 RAM"的坑：
  //  1) ≤ 本机真实 DRAM：超出部分分配器会发出去（写即总线外部中止）；
  //  2) < 传统外设镜像 MMIO_BASE = 0xFC000000（platform/raspi5/gpio.h），
  //     否则 GPIO/UART 寄存器会被当普通内存页分配。
  // 1GB 机型两条都满足。换机型时改这里：2GB→0x80000000、
  // 4GB 及以上→0xF0000000（u32 安全上限；分配器尚未 64 位化：
  // mem->length 截断 u32、boot_info->total_memory 为 i32）。
  /* 【不能从 0 报】bring-up 实测：ya_alloc_init 会把 [0,kernel_base) 也切成
   * 空闲块 ⇒ mm_add_block 在【物理地址 0】写块头，真机直接卡死（QEMU 不检查）。
   * 这段只有 512KB，直接从 kernel_base 报起；ya_alloc_init 的 kernel 切分
   * 逻辑会自动只加 [kernel_end, 1GB) 一块。 */
  ptr->base = KERNEL_BASE;                    // 0x80000
  ptr->length = 0x40000000ULL - KERNEL_BASE;  // 1GB - 512KB
  ptr->type = 1;
  boot_info->total_memory += ptr->length;
  ptr++;
  count++;
#elif defined(RASPI3)
  // Raspberry Pi 3 (BCM2837): DRAM 从 0 开始 1GB，但 0x3F000000-0x40000000
  // 是外设 MMIO 窗口（MMIO_BASE 0x3F000000 + MMIO_LENGTH 0x1000000，见
  // duck/platform/raspi3/gpio.h）——不属于页分配器，绝不能划进 RAM。
  // 可分配区间 = [0, 0x3F000000) = 1008MB。
  // 【原先写 0x40000000 的后果】外设窗口被当成普通 RAM 发出去：分配到该
  // 区间即写坏 UART/GPIO 寄存器；0x40000000 本身更在 DRAM 之外 →
  // 访问触发同步外部中止（总线错误），缺页风暴拖死系统。
  ptr->base = 0x00000000;
  ptr->length = 0x3F000000;  // 1008MB = 1GB - 16MB 外设窗口
  ptr->type = 1;
  boot_info->total_memory += ptr->length;
  ptr++;
  count++;
#else
  ptr->type = 1;
  ptr->base = 0x0000000;
  ptr->length = boot_info->kernel_base;
  boot_info->total_memory += ptr->length;
  ptr++;
  count++;

  ptr->type = 2;
  ptr->base = (u64)boot_info->kernel_base;
  ptr->length = (u64)boot_info->kernel_size;
  boot_info->total_memory += ptr->length;
  ptr++;
  count++;

  ptr->type = 1;
  ptr->base = (u64)boot_info->kernel_base + (u64)boot_info->kernel_size;
  ptr->length = 0xf000000;
  boot_info->total_memory += ptr->length;
  ptr++;
  count++;
#endif

  boot_info->memory_number = count;
}

void init_cpu() {
  // Enable FPU and SIMD for ARM64
  // ARM64 has these enabled by default, but we can configure
}

void read_kernel() {}

void init_boot() {
#ifdef RASPI5
  /* 【最先做的事 · ①】复位信标：前 3 次启动复位整机（数固件日志重来几次
   * 就知道代码有没有跑到这里）；第 4 次继续。详见 boot_beacon 注释。 */
  boot_beacon();

  /* 【最先做的事 · ②】控制台自检：给每个候选口发 3 个自己的字母
   * （AAA/BBB/...）。串口上先出现哪组字母，哪个地址就是活的控制台 ——
   * 一次上电定位，不依赖任何后续初始化（固件刚用完这条串口，它是好的）。 */
  pi5_console_selfid();

  /* 【版本串】用来确认卡上烧的是哪一次构建（改一次构建就 +1）。
   * v6 = 诊断探针全部收掉（kprintf 通路已证实可用），保留真正的修复：
   *      serial10 控制台 + 提前装异常向量 + [EXC] 直写报错 + 栈加大到 32KB。 */
  print_string("YIYIYA-RS5\n\r");
#endif

  // 初始化 data 段（从 FLASH 复制到 RAM）
  init_data();
  
  // Note: BSS is already cleared in boot-armv8-a.s before calling init_boot()
  // Do NOT call init_bss() here to avoid double-clearing

  uart_init();

  init_boot_info();
  uart_send_ch('b');
  uart_send_ch('o');
  uart_send_ch('o');
  uart_send_ch('t');
  uart_send_ch('\n');
  uart_send_ch('\r');
  /* 用 print_hex 而不是 printf("%x")：后者是"读栈当可变参"的老写法，
   * AArch64 上可变参走寄存器 ⇒ 打出来是垃圾（实测 boot info addr 打出一串乱码）。
   * print_hex 的参数是具名 unsigned long，值一定是真的。 */
  print_string("boot info addr ");
  print_hex((unsigned long)boot_info);
  print_string("\n\r");

  print_string("init display\n\r");
  init_display();

  print_string("init memory\n\r");
  init_memory();

  print_string("init disk\n\r");
  init_disk();

  print_string("read kernel\n\r");
  read_kernel();

  print_string("init cpu\n\r");
  init_cpu();

  print_string("start kernel\n\r");
  start_kernel();

  for (;;)
    ;
}

void init_apu_boot() {
  while (boot_info == NULL) {
    asm volatile("wfe" ::: "memory");
  }
  while (boot_info->kernel_entry == NULL) {
    asm volatile("wfe" ::: "memory");
  }
  printf("boot apu info addr %x\n\r", boot_info);
  start_apu_kernel();
  for (;;)
    ;
}

void* memmove64(void* s1, const void* s2, u64 n) {
  u64 *dest, *src;
  int i;
  dest = (u64*)s1;
  src = (u64*)s2;
  for (i = 0; i < n / 8; i++) {
    dest[i] = src[i];
  }
  return s1;
}

static void load_elf(Elf64_Ehdr* elf_header) {
  u8* elf = (u8*)elf_header;
  Elf64_Phdr* phdr = (Elf64_Phdr*)(elf + elf_header->e_phoff);
  u64 entry = 0;

  for (int i = 0; i < elf_header->e_phnum; i++) {
    printf("type:%d\n\r", phdr[i].p_type);
    switch (phdr[i].p_type) {
      case PT_NULL: {
        char* vaddr = (char*)phdr[i].p_vaddr;
        memset(vaddr, 0, phdr[i].p_memsz);
      } break;
      case PT_LOAD: {
        char* start = (char*)(elf + phdr[i].p_offset);
        char* vaddr = (char*)phdr[i].p_vaddr;

        entry = phdr[i].p_vaddr;
        printf("load start:%x vaddr:%x size:%x \n\r", start, vaddr,
               phdr[i].p_filesz);
        memmove64(vaddr, start, phdr[i].p_memsz);
        printf("move end\n\r");

        int num = boot_data.segments_number++;
        boot_data.segments[num].start = vaddr;
        boot_data.segments[num].size = phdr[i].p_memsz;
        boot_data.segments[num].type = 1;
      } break;
      default:
        break;
    }
  }

  Elf64_Shdr* shdr = (Elf64_Shdr*)(elf + elf_header->e_shoff);
  for (int i = 0; i < elf_header->e_shnum; i++) {
    if (SHT_NOBITS == shdr[i].sh_type) {
      char* vaddr = (char*)shdr[i].sh_addr;
      memset(vaddr, 0, shdr[i].sh_size);
    } else if (entry != shdr[i].sh_addr && SHT_PROGBITS == shdr[i].sh_type &&
               shdr[i].sh_flags & SHF_ALLOC && shdr[i].sh_flags) {
      char* start = (char*)(elf + shdr[i].sh_offset);
      char* vaddr = (char*)shdr[i].sh_addr;
      printf("load shdr start:%x vaddr:%x size:%x \n\r", start, vaddr,
             shdr[i].sh_size);
      memmove64(vaddr, start, shdr[i].sh_size);
    }
  }
}

void* load_kernel() {
#ifdef KERNEL_MOVE
  u64* elf = boot_info->kernel_origin_base;
#else
  u64* elf = boot_info->kernel_base;
#endif
  printf("kernel base %x\n\r", elf);

  Elf64_Ehdr* elf_header = (Elf64_Ehdr*)elf;
  if (elf_header->e_ident[0] == ELFMAG0 || elf_header->e_ident[1] == ELFMAG1) {
    load_elf(elf_header);
    return (void*)elf_header->e_entry;
  } else {
    printf("bin kernel\n\r");
    return elf;
  }
}

void start_kernel() {
#ifdef SINGLE_KERNEL
  init_segment();
  extern void kstart(int argc, char* argv[], char** envp);
  entry start = kstart;
  boot_info->kernel_entry = start;
  asm volatile("dsb sy" ::: "memory");
  asm volatile("sev");
  print_string("kernel entry ");
  print_hex((unsigned long)boot_info->kernel_entry);
  print_string("\n\r");
#else
  boot_info->kernel_entry = load_kernel();
  entry start = boot_info->kernel_entry;
  asm volatile("dsb sy" ::: "memory");
  asm volatile("sev");
  print_string("kernel entry ");
  print_hex((unsigned long)boot_info->kernel_entry);
  print_string("\n\r");
#endif
  int argc = 0;
  char** argv = 0;
  char* envp[10];
  envp[0] = (char*)boot_info;
  envp[1] = (char*)(long)cpu_id++;
  start(argc, argv, envp);
}

void start_apu_kernel() {
  printf("start apu kernel entry %x\n\r", boot_info->kernel_entry);
  entry start = boot_info->kernel_entry;
  int argc = 0;
  char** argv = 0;
  char* envp[10];
  envp[0] = (char*)boot_info;
  envp[1] = (char*)(long)cpu_id++;
  start(argc, argv, envp);
}
