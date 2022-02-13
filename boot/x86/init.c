/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "init.h"

// 标识16位代码
asm(".code16gcc\n");
asm("cli\n");
#if defined(__WIN32__)
asm("jmpl $0x0000, $_init_boot\n");
#else
asm("jmpl $0x0000, $init_boot\n");
#endif

boot_info_t* boot_info = NULL;
boot_info_t boot_data;
u8 kernel_stack[1024];  // 1k
u8 kernel_stack_top[0];

// 清屏,设置光标
void cls() {
  // clear screen
  // int 10h ah=06h表滚动窗口 AL=滚动的列数，若为0则实现清空屏幕功能
  asm("int $0x10\n" : : "a"(0x0600));
  // set cursor pos
  // int 10h ah=02h表设定光标位置 DH=游标的列数 DL=游标的行数 BH=页码
  asm("int $0x10\n" : : "a"(0x0200), "b"(0x0000), "d"(0x00));
  // __asm__ __volatile__("int $0x10" : : "a"(0x0013));
}

void print_string(const unsigned char* str) {
  while (*str) {
    asm("int $0x10\n" : : "a"(0x0e00 | *str), "b"(0x0007));
    ++str;
  }
}

void getch() {
  asm("xorw %ax, %ax\n"
      "int $0x16\n");
}

void print_char(char s) {
  // int 10h ah=0eh al=打印字符 bh＝页码 bl=前景色
  asm("int $0x10\n" : : "a"(s | 0x0e00), "b"(0x0007));
}

void itoa(char* buf, int base, int d) {
  char* p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;

  /* If %d is specified and D is minus, put `-' in the head.  */
  if (base == 'd' && d < 0) {
    *p++ = '-';
    buf++;
    ud = -d;
  } else if (base == 'x')
    divisor = 16;

  /* Divide UD by DIVISOR until UD == 0.  */
  do {
    int remainder = ud % divisor;

    *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
  } while (ud /= divisor);

  /* Terminate BUF.  */
  *p = 0;

  /* Reverse BUF.  */
  // 字符顺序调整
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
  // 取得参数列表基址
  char** arg = (char**)&format;
  int c;
  char buf[20];

  arg++;

  while ((c = *format++) != 0) {
    // 为可输出字符
    if (c != '%')
      print_char(c);
    // 分析格式
    else {
      char* p;

      c = *format++;
      switch (c) {
        case 'd':
        case 'u':
        case 'x':
          itoa(buf, c, *((int*)arg++));
          p = buf;
          goto string;
          break;

        case 's':
          p = *arg++;
          if (!p) p = "(null)";

        string:
          while (*p) print_char(*p++);
          break;

        default:
          print_char(*((int*)arg++));
          break;
      }
    }
  }
}

// 初始化boot_info信息
void init_boot_info() {
  boot_info = &boot_data;
  boot_info->version = BOOT_VERSION;
  boot_info->kernel_origin_base = KERNEL_ORIGIN_BASE;
  boot_info->kernel_base = KERNEL_BASE;
  boot_info->kernel_size = KERNEL_BLOCK_SIZE * READ_BLOCK_SIZE * 2;
  boot_info->gdt_number = GDT_NUMBER;
  boot_info->pdt_base = PDT_BASE;
  boot_info->tss_number = MAX_CPU;
}

void init_disk() {
  // 固定值
  boot_info->disk.hpc = 2;
  // 每柱面扇区数
  boot_info->disk.spt = 18;
  // 1.44m软盘
  boot_info->disk.type = 1;  // 1.44m floppy flat lba
}

void init_display() {
  boot_info->disply.mode = 1;
  boot_info->disply.video = 0xB8000;
  boot_info->disply.height = 25;
  boot_info->disply.width = 80;
}

void* memmove(void* s1, const void* s2, u32 n) {
  // char *dest, *src;
  // u32 i;
  // dest = (char*)s1;
  // src = (char*)s2;
  // for (i = 0; i < n; i++) dest[i] = src[i];
  u16 *dest, *src;
  int i;
  dest = (u16*)s1;
  src = (u16*)s2;
  for (i = 0; i < n / 2; i++) {
    dest[i] = src[i];
  }
  return s1;
}

u8 disk_read(u16 disk, u16 head, u16 cylinder, u16 sector, u16 number, u32 addr,
             u8* status) {
  u8 carry = 0;
  u16 cx =
      ((cylinder & 0xff) << 8) | (sector & 0xff);
  u16 bx = (addr & 0xffff);
  u16 es = ((addr >> 4) & 0xf000);
  u16 dx = (head << 8) | disk;
  u16 ax = (0x02 << 8) | number;
  u16 erro;
  // int 13h ah=02h 读取磁盘扇区 al=读入的扇区数 ch=磁道号的低8位 cl=扇区号1~63(bit 0~5)磁道号,(柱面号)的高2位(bit 6~7,只对硬盘有效) dh=磁头号 dl=驱动器号 es:bx=>数据缓冲区
  // 调用结束 如果访问出错则CF被置1，AX存放错误信息 如果访问成功则CF被清零，AH存放状态，AL存放读入扇区数
  asm volatile(
      "pushw %%es \n"
      "movw %w[seg], %%es \n"
      "stc\n\t"
      "int $0x13  \n"
      "sti\n\t"
      "popw %%es\n"
      "setc	%[cf] \n"
      : [cf] "=qm"(carry), "=a"(erro)
      : "a"(ax), "b"(bx), "d"(dx), [seg] "r"(es), "c"(cx)
      : "memory", "cc");
  *status = erro;
  return carry;
}

u8 disk_read_lba(u32 lba, u32 addr, u8* status) {
  // 将LBA格式转换为CHS格式（Cylinder/Head/Sector，柱面/磁头/扇区）
  u32 cylinder = lba / boot_info->disk.spt / boot_info->disk.hpc;
  u32 head = (lba / boot_info->disk.spt) % boot_info->disk.hpc;
  u32 sector = (lba % boot_info->disk.spt) + 1;

  // todo read by disk type
  //printf("addr:%x lba:%d CHS = (%d, %d, %d)\r\n", addr,lba,cylinder,head,sector);
  u8 ret = disk_read(0, head, cylinder, sector, 1, addr, status);
  if (ret == 0) {
    // printf("addr %x ", addr);
  } else {
    printf("\n\rread erro on lba:%x cylinder:%d head:%d sector:%d\n\r", lba,
           cylinder, head, sector);
  }
  return ret;
}

void read_kernel() {
  u8 status = 0;
  print_string("");
  // 柱面号
  u32 cylinder = 0;
  // 磁头号
  u32 head = 0;
  // 扇区号
  u32 sector = 0xc;
  // LBA（Logical Block Address，逻辑块寻址）格式磁盘扇区号
  u32 lba = 0x00;
  u8 ret = 0;
  if (boot_info->disk.type == 1) {
    lba = (cylinder * boot_info->disk.hpc + head) * boot_info->disk.spt +
          (sector);
// lba=sector;
// flopy 1.44m
#ifdef KERNEL_MOVE
    u32 addr = boot_info->kernel_origin_base;
#else
    // 内核将要被读取到的位置
    u32 addr = boot_info->kernel_base;
#endif
    for (int i = 0; i < KERNEL_BLOCK_SIZE * 2; i++) {
      // if(i>=20){
      //   u32 *p=0xffff;
      //   printf("i=%d addr=%x\n\r",i,addr);
      //   debugger;
      //   *p=0x1234;
      //   debugger;
      // }

      // disk sector number
      ret = disk_read_lba(lba, addr, &status);
      if (ret == 0) {
        print_string(".");
      } else {
        printf("\n\rload kernel offset %d error code %d addr:%x\n\r", i, status,
               addr);
        break;
      }
      lba++;
      addr += READ_BLOCK_SIZE;
    }
  }
  if (ret == 0) {
    print_string("\n\rload kernel success\n\r");
  } else {
    print_string("\n\rload kernel error\n\r");
    for (;;)
      ;
  }
}

static inline void enable_a20() {
  asm("inb $0x92,%al \n"
      "or $2,%al \n"
      "outb %al,$0x92 \n");
}

int memory_prob() {
  u32 id = 0;
  int count = 0, signature, bytes;
  memory_info_t* ptr = boot_info->memory;
  boot_info->total_memory = 0;
  for (; count < MAX_MEMORY_BLOCK;) {
    // int 15h ax=E820h表获取内存信息 ebx第一次调用必须为0 ecx表内存结构体的大小(字节) edx=0534D4150h es:di为内存信息结构体
    // 调用结束 eax=0534D4150h(如果不为0534D4150h，则硬件故障) ebx=无需关心值为什么，只需再次传入即可获取下一个内存信息描述符 ecx=被填充的字符数
    asm("int  $0x15"
        : "=a"(signature), "=c"(bytes), "=b"(id)
        : "a"(0xE820), "b"(id), "c"(24), "d"(0x534D4150), "D"(ptr));
    // 硬件故障
    if (signature != 0x534D4150) {
      return -1;
    } else if (bytes > 20 && (ptr->extended & 0x0001) == 0) { // 检查CF，是否发生错误
    } else {
      boot_info->total_memory += ptr->length;
      ptr++;
      count++;
    }
    // ebx为0表示为最后一个内存区域
    if (id == 0) {
      break;
    }
  }
  // 记录内存区域数量
  boot_info->memory_number = count;
  return count;
}
void init_memory() {
  // read memory info
  enable_a20();
  memory_prob();
  // page setup
}

static inline unsigned long read_cr0(void) {
  unsigned long val;
  asm volatile("movl %%cr0, %0" : "=r"(val) : : "memory");
  return val;
}

static inline void write_cr0(unsigned long val) {
  asm volatile("movl %0, %%cr0" : : "r"(val) : "memory");
}

void init_gdt() {
  u64* gdt_addr = boot_info->gdt_base;
  printf("gdt_addr=%x\n\r", gdt_addr);
  gdt_addr[GDT_ENTRY_NULL] = 0;
  gdt_addr[GDT_ENTRY_32BIT_CS] = GDT_ENTRY(0, 0xfffff, 0xc09b);  // 0x8
  gdt_addr[GDT_ENTRY_32BIT_DS] = GDT_ENTRY(0, 0xfffff, 0xc092);  // 0x10
  gdt_addr[GDT_ENTRY_32BIT_FS] = GDT_ENTRY(0, 0xfffff, 0xc093);  // 0x18

  u32 tss_base = (u32) & (boot_info->tss[0]);
  gdt_addr[GDT_ENTRY_32BIT_TSS] = GDT_ENTRY(tss_base, 0xfffff, 0xc089);  // 0x20
  gdt_addr[GDT_ENTRY_USER_32BIT_CS] =
      GDT_ENTRY(0, 0xfffff, 0xc09b | GDT_DPL(3));  // 0x28
  gdt_addr[GDT_ENTRY_USER_32BIT_DS] =
      GDT_ENTRY(0, 0xfffff, 0xc092 | GDT_DPL(3));  // 0x30
  gdt_addr[GDT_ENTRY_USER_32BIT_FS] =
      GDT_ENTRY(0, 0xfffff, 0xc092 | GDT_DPL(3));  // 0x38

  gdt_addr[GDT_ENTRY_16BIT_CS] = GDT_ENTRY(0, 0xfffff, 0x009b);
  gdt_addr[GDT_ENTRY_16BIT_DS] = GDT_ENTRY(0, 0xfffff, 0x0093);

  // load gdt
  gdt_ptr_t gdt;
  gdt.limit = (boot_info->gdt_number * GDT_SIZE) - 1;
  gdt.base = (u32)gdt_addr;
  asm volatile("lgdtl %0\n" : : "m"(gdt));

  // asm volatile("movl %0, %%ss" : : "r"(GDT_ENTRY_16BIT_DS * GDT_SIZE));
  // asm volatile("movl %0, %%ds" : : "r"(GDT_ENTRY_16BIT_DS * GDT_SIZE));
  // asm volatile("movl %0, %%es" : : "r"(GDT_ENTRY_16BIT_DS * GDT_SIZE));
  // asm volatile("movl %0, %%gs" : : "r"(GDT_ENTRY_16BIT_DS * GDT_SIZE));
  // asm volatile("movl %0, %%fs" : : "r"(GDT_ENTRY_16BIT_DS * GDT_SIZE));
  // debugger;
  // asm volatile("movl %0, %%cs" : : "r"(GDT_ENTRY_16BIT_CS * GDT_SIZE));
  // asm("jmpl %0,$here\n"
  //     "  here:\n" ::"i"(GDT_ENTRY_16BIT_CS * GDT_SIZE));
}

void init_cpu() {
  // enable cr0
  // 置位CR0寄存器的第0位来开启保护模式
  unsigned long cr0 = read_cr0();
  write_cr0(cr0 | 1);
}

void init_boot() {
  // 初始化boot_info信息
  init_boot_info();
  
  // 清屏,设置光标
  cls();
  printf("boot info addr %x\n\r", boot_info);
  
  // 初始化gdt
  init_gdt();
  
  // 初始化显示模式信息
  print_string("init display\n\r");
  init_display();
  
  // 初始化内存信息
  print_string("init memory\n\r");
  init_memory();
  
  // 初始化软盘
  init_disk();
  
  // 读取内核
  read_kernel();
  
  // 初始化cpu
  init_cpu();
  
  // 栈设置
  asm volatile(
      "movl %0, %%esp\n"
      "mov %%esp,%%ebp\n"
      :
      : "m"(kernel_stack_top));\
      
#if defined(__WIN32__)      
  asm("jmpl %0, $_start_kernel" ::"i"(GDT_ENTRY_32BIT_CS * GDT_SIZE));
#else
  asm("jmpl %0, $start_kernel" ::"i"(GDT_ENTRY_32BIT_CS * GDT_SIZE));

#endif

  for (;;)
    ;
}

asm(".code32\n");

void* load_kernel() {
#ifdef KERNEL_MOVE
  u32* elf = boot_info->kernel_origin_base;
#else
  u32* elf = boot_info->kernel_base;
#endif

  Elf32_Ehdr* elf_header = (Elf32_Ehdr*)elf;
  // 判断魔数,如果是elf文件,则加载elf文件，找到内核程序入口
  if (elf_header->e_ident[0] == ELFMAG0 || elf_header->e_ident[1] == ELFMAG1) {
    // printf("header: ");
    // for (int x = 0; x < 16; x++) {
    //   printf("%x ", elf_header->e_ident[x]);
    // }
    // printf("\n\r");

    // printf("type:%d\n\r", elf_header->e_type);
    // printf("e_machine:%d\n\r", elf_header->e_machine);
    // printf("e_entry:%d\n\r", elf_header->e_entry);
    // printf("e_phoff:%d\n\r", elf_header->e_phoff);
    // printf("e_shoff:%d\n\r", elf_header->e_shoff);
    // printf("e_ehsize:%d\n\r", elf_header->e_ehsize);
    // printf("e_phentsize:%d\n\r", elf_header->e_phentsize);
    load_elf(elf_header);
    return elf_header->e_entry;
  } else {
    // printf("bin kernel\n\r");  
    return boot_info->kernel_base;  // 如果不是elf文件，程序入口即内核地址
  }
}

// 从s2复制n个字符到s1
void* memmove32(void* s1, const void* s2, u32 n) {
  u32 *dest, *src;
  int i;
  dest = (u32*)s1;
  src = (u32*)s2;
  for (i = 0; i < n / 4; i++) {
    dest[i] = src[i];
  }
}

// 处理elf文件
void load_elf(Elf32_Ehdr* elf_header) {
  // printf("e_phnum:%d\n\r", elf_header->e_phnum);
  u16* elf = elf_header;
  Elf32_Phdr* phdr = (elf + elf_header->e_phoff / 2);
  // printf("addr %x elf=%x\n\r", phdr, elf);

  for (int i = 0; i < elf_header->e_phnum; i++) {
    // printf("type:%d\n\r", phdr[i].p_type);
    switch (phdr[i].p_type) {
      case PT_NULL:
        // printf(" %s %x %x %x %s %x %x \r\n", "NULL", phdr[i].p_offset,
        //        phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
        //        phdr[i].p_memsz);
        break;
      // 提取出来可加载程序段
      case PT_LOAD: {
        // printf(" %s %x %x %x %s %x %x \r\n", "LOAD", phdr[i].p_offset,
        //        phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
        //        phdr[i].p_memsz);
        char* start = elf + phdr[i].p_offset / 2;
        char* vaddr = phdr[i].p_vaddr;
        // printf("load start:%x vaddr:%x size:%x \n\r", start, vaddr,
        //        phdr[i].p_filesz);
        memmove32(vaddr, start, phdr[i].p_filesz);
        // printf("move end\n\r");
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
}

// start kernel
void start_kernel() {
  asm volatile("cli\n");
  // 寄存器置位
  asm volatile("movl %0, %%ss" : : "r"(GDT_ENTRY_32BIT_DS * GDT_SIZE));
  asm volatile("movl %0, %%ds" : : "r"(GDT_ENTRY_32BIT_DS * GDT_SIZE));
  asm volatile("movl %0, %%es" : : "r"(GDT_ENTRY_32BIT_DS * GDT_SIZE));
  asm volatile("movl %0, %%gs" : : "r"(GDT_ENTRY_32BIT_DS * GDT_SIZE));
  asm volatile("movl %0, %%fs" : : "r"(GDT_ENTRY_32BIT_FS * GDT_SIZE));
  // print_string("load kernel\n\r");
  // 获取内核入口
  boot_info->kernel_entry = load_kernel();
  entry start = boot_info->kernel_entry;
  
  // 参数
  int argc = 0;
  char** argv = 0;
  char* envp[10];
  envp[0] = boot_info;
  
  // 转至内核
  start(argc, argv, envp);
}
