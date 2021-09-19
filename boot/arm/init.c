/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "init.h"
#include "gpio.h"


boot_info_t* boot_info = NULL;
boot_info_t boot_data;
typedef int (*entry)(int, char**, char**);

void io_write32(uint port, u32 data) {
  *(u32 *) port = data;
}

u32 io_read32(uint port) {
  u32 data;
  data=*(u32 *) port;
  return data;
}

void cls() {}

#ifdef V3S
void init_uart(){
  u32 addr;
	u32 val;
	/* Config GPIOB8 and GPIOB9 to txd0 and rxd0 */
	addr = 0x01c20824 + 0x04;
	val = io_read32(addr);
	val &= ~(0xf << ((8 & 0x7) << 2));
	val |= ((0x3 & 0x7) << ((8 & 0x7) << 2));
	io_write32(addr, val);

	val = io_read32(addr);
	val &= ~(0xf << ((9 & 0x7) << 2));
	val |= ((0x3 & 0x7) << ((9 & 0x7) << 2));
	io_write32(addr, val);

	/* Open the clock gate for uart0 */
	addr = 0x01c2006c;
	val = io_read32(addr);
	val |= 1 << 16;
	io_write32(addr, val);

	/* Deassert uart0 reset */
	addr = 0x01c202d8;
	val = io_read32(addr);
	val |= 1 << 16;
	io_write32(addr, val);

	/* Config uart0 to 115200-8-1-0 */
	addr = 0x01c28000;
	io_write32(addr + 0x04, 0x0);
	io_write32(addr + 0x08, 0xf7);
	io_write32(addr + 0x10, 0x0);
	val = io_read32(addr + 0x0c);
	val |= (1 << 7);
	io_write32(addr + 0x0c, val);
	io_write32(addr + 0x00, 0xd & 0xff);
	io_write32(addr + 0x04, (0xd >> 8) & 0xff);
	val = io_read32(addr + 0x0c);
	val &= ~(1 << 7);
	io_write32(addr + 0x0c, val);
	val = io_read32(addr + 0x0c);
	val &= ~0x1f;
	val |= (0x3 << 0) | (0 << 2) | (0x0 << 3);
	io_write32(addr + 0x0c, val);

}

void uart_send(unsigned int c) {
  u32 addr = 0x01c28000;
	while((io_read32(addr + 0x7c) & (0x1 << 1)) == 0);
	io_write32(addr + 0x00, c);
}

char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(*UART0_FR&0x10);
    /* read it and return */
    r=(char)(*UART0_DR);
    /* convert carrige return to newline */
    return r=='\r'?'\n':r;
}

#else
void init_uart() {
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
  *UART0_LCRH = 0b11 << 5;  // 8n1
  *UART0_CR = 0x301;        // enable Tx, Rx, FIFO

}

void uart_send(unsigned int c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(*UART0_FR&0x20);
    /* write the character to the buffer */
    *UART0_DR=c;
}

char uart_getc() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(*UART0_FR&0x10);
    /* read it and return */
    r=(char)(*UART0_DR);
    /* convert carrige return to newline */
    return r=='\r'?'\n':r;
}
#endif

void print_string(const unsigned char* str) {
  while (*str) {
    uart_send(*str);
    ++str;
  }
}

void getch() {
  uart_getc();
}

void print_char(char s) {
  uart_send(s);
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
      print_char(c);
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

volatile unsigned char* const UART0_PTR = (unsigned char*)0x0101f1000;

void display(const char* string) {
  while (*string != '\0') {
    *UART0_PTR = *string;
    string++;
  }
}

void init_boot_info() {
  boot_info = &boot_data;
  boot_info->version = BOOT_VERSION;
  boot_info->kernel_origin_base = KERNEL_ORIGIN_BASE;
  boot_info->kernel_base = KERNEL_BASE;
  boot_info->kernel_size = KERNEL_BLOCK_SIZE * READ_BLOCK_SIZE * 2;
  boot_info->tss_number = MAX_CPU;
}

void init_disk() {
  boot_info->disk.hpc = 2;
  boot_info->disk.spt = 18;
  boot_info->disk.type = 1;  // 1.44m floppy flat lba
}

void init_display() {
  boot_info->disply.mode = 1;
  boot_info->disply.video = 0xB8000;
  boot_info->disply.height = 25;
  boot_info->disply.width = 80;
}

void init_memory() {
  // read memory info
  // enable_a20();
  
  int count = 0;
  memory_info_t* ptr = boot_info->memory;
  boot_info->total_memory = 0;
  #ifdef V3S
    // ptr->base=0x00000;
    // ptr->length=0xEFFF;
    // ptr->type=1;
    // boot_info->total_memory += ptr->length;
    // ptr++;
    // count++;
    ptr->base  =0x40000000; 
    ptr->length=0x1000000*4;//16M*4
    ptr->type=1;
    boot_info->total_memory += ptr->length;
    ptr++;
    count++;

  #else
      //
      ptr->type=1;
      ptr->base=0x00000000;
      ptr->length=boot_info->kernel_base; 
      boot_info->total_memory += ptr->length;
      ptr++;
      count++;
      
      ptr->type=1;
      ptr->base=(u32)boot_info->kernel_base+(u32)boot_info->kernel_size;
      ptr->length=0xf000000; //64M
      boot_info->total_memory += ptr->length;
      ptr++;
      count++;
  #endif
  boot_info->memory_number = count;
  // page setup
}
void init_cpu() {
  // enable cr0
}

void read_kernel() {

}

void init_boot() {
  init_uart();

  display("hello duck\n");
  init_boot_info();
  display("111\n");
  cls();
  
  printf("boot info addr %x\n\r", boot_info);

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

void* memset(void* s, int c, size_t n) {
  int i;
  for (i = 0; i < n; i++) ((char*)s)[i] = c;
  return s;
}

void* memmove32(void* s1, const void* s2, u32 n) {
  u32 *dest, *src;
  int i;
  dest = (u32*)s1;
  src = (u32*)s2;
  for (i = 0; i < n / 4; i++) {
    dest[i] = src[i];
  }
}

void load_elf(Elf32_Ehdr* elf_header) {
  // printf("e_phnum:%d\n\r", elf_header->e_phnum);
  u16* elf = elf_header;
  Elf32_Phdr* phdr = (elf + elf_header->e_phoff/2 );
  // printf("addr %x elf=%x\n\r", phdr, elf);
  u32 entry=0;
  for (int i = 0; i < elf_header->e_phnum; i++) {
    printf("type:%d\n\r", phdr[i].p_type);
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
        char* start = elf + phdr[i].p_offset /2;
        char* vaddr = phdr[i].p_vaddr;
        entry=vaddr;
        printf("load start:%x vaddr:%x size:%x \n\r", start, vaddr,
               phdr[i].p_filesz);
        memmove32(vaddr, start, phdr[i].p_memsz);
        printf("move end\n\r");
      } break;
      default:
        break;
    }
  }

  Elf32_Shdr* shdr = (elf + elf_header->e_shoff/2 );;
  for (int i = 0; i < elf_header->e_shnum; i++) {
    if (SHT_NOBITS == shdr[i].sh_type) {
      char* vaddr = shdr[i].sh_addr;
      memset(vaddr,0,shdr[i].sh_size);
      // map_alignment(page,vaddr,buf,shdr[i].sh_size);
    } else if (entry != shdr[i].sh_addr &&
               SHT_PROGBITS == shdr[i].sh_type &&
               shdr[i].sh_flags & SHF_ALLOC && shdr[i].sh_flags) {
      char* start = shdr[i].sh_offset;
      char* vaddr = shdr[i].sh_addr;
      printf("load shdr start:%x vaddr:%x size:%x \n\r", start, vaddr,
              shdr[i].sh_size);
      u32 *phstart=(u32)elf+shdr[i].sh_offset;
      memset(vaddr,0,shdr->sh_size);
      memmove32(phstart,vaddr,shdr[i].sh_size);
    }
  }

}

void print_hex(u32* addr){
   for (int x = 0; x < 16; x++) {
      printf("%x ", addr[x]);
    }
    printf("\n\r");
}

void* load_kernel() {
   #ifdef KERNEL_MOVE
  u32* elf = boot_info->kernel_origin_base;
#else
  u32* elf = boot_info->kernel_base;
#endif
  printf("kernel base %x\n\r",elf);
  //print_hex(elf);
  printf("\n\r");
  Elf32_Ehdr* elf_header = (Elf32_Ehdr*)elf;
  if (elf_header->e_ident[0] == ELFMAG0 || elf_header->e_ident[1] == ELFMAG1) {
    // printf("header: ");
   

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
    printf("bin kernel\n\r");
    return elf;
  }

}


// start kernel
void start_kernel() {
  boot_info->kernel_entry = load_kernel();
  entry start = boot_info->kernel_entry;
  printf("kernel entry %x\n\r",boot_info->kernel_entry);
  // print_hex(boot_info->kernel_entry);
  
  int argc = 0;
  char** argv = 0;
  char* envp[10];
  envp[0] = boot_info;
  start(argc, argv, envp);
}
