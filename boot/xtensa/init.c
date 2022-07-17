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
typedef void (*rom_write_char_uart_fn)(char c);
typedef u32 (*rom_spiflash_read_fn)(u32 src, u32* des, u32 len);
typedef int (*rom_printf_fn)(const char* fmt, ...);
typedef unsigned int (*rom_cache_flash_mmu_set_fn)(int cpu_no, int pid, unsigned int vaddr, unsigned int paddr,  int psize, int num);

typedef void (*rom_cache_read_enable_fn)(int cpu_no);

volatile unsigned char* const UART0_PTR = (unsigned char*)0x0101f1000;

rom_printf_fn printf = 0x40007d54;
rom_spiflash_read_fn disk_read_lba = 0x40062ed8;
rom_write_char_uart_fn send = 0x40007cf8;
rom_cache_flash_mmu_set_fn cache_flash_mmu_set= 0x400095e0;
rom_cache_read_enable_fn cache_read_enable= 0x40009a84;


extern int _bss_start;
extern int _bss_end;
extern int _data_start;
extern int _data_end;

void io_write32(uint port, u32 data) { *(u32*)port = data; }

u32 io_read32(uint port) {
  u32 data;
  data = *(u32*)port;
  return data;
}

void cls() {}

void init_uart() {
  u32 addr;
  u32 val;
}

void uart_send(u8 c) { send(c); }

char uart_getc() {
  // char r;
  // /* wait until something is in the buffer */
  // do{asm volatile("nop");}while(*UART0_FR&0x10);
  // /* read it and return */
  // r=(char)(*UART0_DR);
  // /* convert carrige return to newline */
  // return r=='\r'?'\n':r;
}

void print_string(const unsigned char* str) {
  while (*str) {
    uart_send(*str);
    ++str;
  }
}

void getch() { uart_getc(); }

void print_char(char s) { uart_send(s); }

void display(const char* string) { print_string(string); }

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
  boot_info->disk.type = 2;  // flash
}

void init_display() {
  boot_info->disply.mode = 1;
  boot_info->disply.video = 0xB8000;
  boot_info->disply.height = 25;
  boot_info->disply.width = 80;
}

void init_memory() {
  // read memory info
  int count = 0;
  memory_info_t* ptr = boot_info->memory;
  boot_info->total_memory = 0;

  ptr->base = 0x3FFF0000;
  ptr->length = 0x100000;  // 16M
  ptr->type = 1;
  boot_info->total_memory += ptr->length;
  ptr++;
  count++;

  boot_info->memory_number = count;
  // page setup
}
void init_cpu() {
  // enable cr0
}

void read_kernel() {
  // #ifdef KERNEL_MOVE
  //   u32 addr = boot_info->kernel_origin_base;
  // #else
  //   u32 addr = boot_info->kernel_base;
  // #endif

  //   u32 read_addr = KERNEL_FLASH_ADDR;
  //   printf("read kernel from flash %x to ram %x\n", read_addr, addr);
  //   u32 ret = disk_read_lba(read_addr, addr, KERNEL_SIZE);
  //   if (ret == 0) {
  //     printf("read kernel success\n");
  //   } else {
  //     printf("read kernel faild\n");
  //   }
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

void init_boot() {
  memset(&_bss_start, 0, (&_bss_end - &_bss_start) * sizeof(_bss_start));

  init_uart();

  display("hello duck\n");
  init_boot_info();
  display("init boot info end\n");
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

#define read16(addr) ((*(u32*)addr) & 0xff)

void load_elf(Elf32_Ehdr* elf_header) {
  u32 e_phnum = read16(&elf_header->e_phnum);
  printf("e_phnum:%d\n", e_phnum);
  u32 elf = KERNEL_FLASH_ADDR;
  Elf32_Phdr phdr_data[PHDR_NUM];
  Elf32_Phdr* phdr = (elf + elf_header->e_phoff);
  disk_read_lba(phdr, &phdr_data, sizeof(Elf32_Phdr) * PHDR_NUM);
  phdr = &phdr_data;
  // printf("addr %x elf=%x\n\r", phdr, elf);
  u32 entry = 0;
  for (int i = 0; i < e_phnum; i++) {
    printf("elf type:%d\n\r", phdr[i].p_type);
    switch (phdr[i].p_type) {
      case PT_NULL:
        printf(" %s %x %x %x %s %x %x \r\n", "NULL", phdr[i].p_offset,
               phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
               phdr[i].p_memsz);
        break;
      case PT_LOAD: {
        if((phdr[i].p_flags & PF_X) == PF_X){//is code
          printf(" %s %x %x %x %s %x %x flag:%x\r\n", "LOAD", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz,phdr[i].p_flags);
          char* start = elf + phdr[i].p_offset;
          char* vaddr = phdr[i].p_vaddr;
          entry = vaddr;
          printf("  init load code start:%x vaddr:%x size:%x \n\r", start, vaddr,
                phdr[i].p_filesz);
          //u32 ret = disk_read_lba(start, vaddr, phdr[i].p_memsz);

          u32 irom_load_addr_aligned = (u32)vaddr & MMU_FLASH_MASK;
          u32 irom_page_count=(phdr[i].p_memsz + ((u32)vaddr - (((u32)vaddr) & MMU_FLASH_MASK)) + MMU_BLOCK_SIZE - 1) / MMU_BLOCK_SIZE;
          int rc = cache_flash_mmu_set(0, 0,irom_load_addr_aligned , ((u32)start) & MMU_FLASH_MASK, 64, irom_page_count);

          rc |= cache_flash_mmu_set(1, 0, irom_load_addr_aligned, ((u32)start) & MMU_FLASH_MASK, 64, irom_page_count);

          printf("  move end code  load addr %x  from %x ret=%d\n\r",irom_load_addr_aligned,start,rc);
        }else if((phdr[i].p_flags & PF_W) == PF_W ){ //is data for write
           printf(" %s %x %x %x %s %x %x flag:%x\r\n", "LOAD", phdr[i].p_offset,
                phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                phdr[i].p_memsz,phdr[i].p_flags);
          char* start = elf + phdr[i].p_offset;
          char* vaddr = phdr[i].p_vaddr;
          entry = vaddr;
          printf("  init load data start:%x vaddr:%x size:%x \n\r", start, vaddr,
                phdr[i].p_filesz);
          //u32 ret = disk_read_lba(start, vaddr, phdr[i].p_memsz);

          u32 drom_load_addr_aligned = (u32)vaddr & MMU_FLASH_MASK;
          u32 drom_page_count=(phdr[i].p_memsz + ((u32)vaddr - (((u32)vaddr) & MMU_FLASH_MASK)) + MMU_BLOCK_SIZE - 1) / MMU_BLOCK_SIZE;
          int rc = cache_flash_mmu_set(0, 0,drom_load_addr_aligned , ((u32)start) & MMU_FLASH_MASK, 64, drom_page_count);

          rc |= cache_flash_mmu_set(1, 0, drom_load_addr_aligned, ((u32)start) & MMU_FLASH_MASK, 64, drom_page_count);

          printf("  move end data load addr %x  from %x ret=%d\n\r",drom_load_addr_aligned,start,rc);
        }
        
      } break;
      default:
        break;
    }
  }

  Elf32_Shdr shdr_data[SHDR_NUM];
  Elf32_Shdr* shdr = (elf + elf_header->e_shoff);
  disk_read_lba(shdr, &shdr_data, sizeof(Elf32_Shdr) * SHDR_NUM);
  shdr = &shdr_data;

  u32 e_shnum = read16(&elf_header->e_shnum);
  printf("e_shnum:%d\n", e_shnum);
  for (int i = 0; i < e_shnum; i++) {
    if (SHT_NOBITS == shdr[i].sh_type) {
      char* vaddr = shdr[i].sh_addr;
      memset(vaddr, 0, shdr[i].sh_size);
      // map_alignment(page,vaddr,buf,shdr[i].sh_size);
    } else if (entry != shdr[i].sh_addr && SHT_PROGBITS == shdr[i].sh_type &&
               shdr[i].sh_flags & SHF_ALLOC && shdr[i].sh_flags) {
      char* start = shdr[i].sh_offset;
      char* vaddr = shdr[i].sh_addr;
      printf("init load shdr start:%x vaddr:%x size:%x \n\r", start, vaddr,
             shdr[i].sh_size);
      u32* phstart = (u32)elf + shdr[i].sh_offset;
      u32 ret = disk_read_lba(phstart, vaddr, shdr[i].sh_size);
      // memset(vaddr, 0, shdr->sh_size);
      // memmove32(phstart, vaddr, shdr[i].sh_size);
    }
  }
}

void print_hex(u32* addr) {
  for (int x = 0; x < 16; x++) {
    printf("%x ", addr[x]);
  }
  printf("\n\r");
}

void* load_kernel() {
#ifdef KERNEL_BIN
  printf("bin kernel\n\r");
  return elf;
#else
  Elf32_Ehdr header;
  Elf32_Ehdr* elf_header = (Elf32_Ehdr*)&header;
  disk_read_lba(KERNEL_FLASH_ADDR, elf_header, sizeof(Elf32_Ehdr));
  printf("parse elf kernel header %x\n", elf_header);
  u32 magic = *(u32*)&elf_header->e_ident;
  printf("elf magic %x\n", magic);
  if (magic == 0x464c457f) {
    // printf("header: ");
    // printf("type:%d\n\r", *(u32*)&elf_header->e_type);
    // printf("e_machine:%d\n\r", elf_header->e_machine);
    // printf("e_entry:%x\n\r", elf_header->e_entry);
    // printf("e_phoff:%x\n\r", elf_header->e_phoff);
    // printf("e_shoff:%x\n\r", elf_header->e_shoff);
    // printf("e_ehsize:%x\n\r", elf_header->e_ehsize);
    // printf("e_phentsize:%x\n\r", elf_header->e_phentsize);
    load_elf(elf_header);
    return elf_header->e_entry;
  } else {
    printf("bin kernel\n");
    return KERNEL_BASE;
  }
#endif
}

// start kernel
void start_kernel() {  
  boot_info->kernel_entry = load_kernel();
  entry start = boot_info->kernel_entry;
  printf("kernel entry %x\n", boot_info->kernel_entry);

  DPORT_REG_CLR_BIT(DPORT_PRO_CACHE_CTRL1_REG,
			(DPORT_PRO_CACHE_MASK_IRAM0) |
			(DPORT_PRO_CACHE_MASK_IRAM1 & 0) |
			(DPORT_PRO_CACHE_MASK_IROM0 & 0) |
			DPORT_PRO_CACHE_MASK_DROM0 |
			DPORT_PRO_CACHE_MASK_DRAM1);

	DPORT_REG_CLR_BIT(DPORT_APP_CACHE_CTRL1_REG,
			(DPORT_APP_CACHE_MASK_IRAM0) |
			(DPORT_APP_CACHE_MASK_IRAM1 & 0) |
			(DPORT_APP_CACHE_MASK_IROM0 & 0) |
			DPORT_APP_CACHE_MASK_DROM0 |
			DPORT_APP_CACHE_MASK_DRAM1);

	cache_read_enable(0);
  cache_read_enable(1);

  int argc = 0;
  char** argv = 0;
  char* envp[4];
  envp[0] = boot_info;
  start(argc, argv, envp);
}
