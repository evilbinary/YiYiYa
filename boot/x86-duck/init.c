#include "init.h"

#include <SDL.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "stdio.h"

#define true 1
#define false 0

#ifdef X86
#define WIDTH 1024
#define HEIGHT 768
#else

#define WIDTH 480
#define HEIGHT 272
#endif

boot_info_t* boot_info = NULL;
boot_info_t boot_data;
u8 kernel_stack[1024];  // 1k
u8 kernel_stack_top[0];
int cpu_id = 0;
FILE* disk_file = NULL;

void cls() {}

void init_cpu() {}

void init_gdt() {
  u64* gdt_addr = boot_info->gdt_base;
  printf("gdt_addr=%x\n\r", gdt_addr);
  gdt_addr[GDT_ENTRY_NULL] = 0;
  gdt_addr[GDT_ENTRY_32BIT_CS] = GDT_ENTRY(0, 0xfffff, 0xc09b);  // 0x8
  gdt_addr[GDT_ENTRY_32BIT_DS] = GDT_ENTRY(0, 0xfffff, 0xc092);  // 0x10
  gdt_addr[GDT_ENTRY_32BIT_FS] = GDT_ENTRY(0, 0xfffff, 0xc093);  // 0x18

  u32 tss_base = (u32) & (boot_info->tss[cpu_id]);
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
  //   asm volatile("lgdtl %0\n" : : "m"(gdt));

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

void print_string(const unsigned char* str) { printf(str); }

void init_memory() {
  // read memory info
  //   enable_a20();
  //   memory_prob();
  // page setup

#ifdef KERNEL_MOVE
  void* addr = boot_info->kernel_origin_base;
#else
  void* addr = boot_info->kernel_base;
#endif
  map_segment("load_mmap", addr, (u32)boot_info->kernel_size * 4);
  // boot_info->kernel_origin_base=malloc(((u32)boot_info->kernel_size) * 4);
  // boot_info->kernel_base=boot_info->kernel_origin_base;
}

void map_segment(char* name, void* addr, u32 size) {
  printf("===>mmap start to %s at %p size: 0x%x\n", name, addr, size);
  int fd = -1;
  if (access(name, 0) == 0) {
    fd = open(name, O_RDWR, 0777);
    printf("file exist\n");
  } else {
    fd = open(name, O_RDWR | O_CREAT, 0777);
    ftruncate(fd, size);
    printf("file not exist\n");
  }

  void* res = mmap(addr, size, PROT_READ | PROT_WRITE | PROT_EXEC,
                   MAP_PRIVATE | MAP_FIXED, fd, 0);  // MAP_ANONYMOUS
  if (res != MAP_FAILED) {
    printf("mmap memory at %p ret=%p\n", addr, res);
  } else {
    printf("mmap faild %s %d %s\n", name, errno, strerror(errno));
    exit(-1);
    // boot_info->kernel_origin_base = malloc(KERNEL_BLOCK_SIZE *
    // READ_BLOCK_SIZE); boot_info->kernel_base = boot_info->kernel_origin_base;
    // printf("malloc memory at %x\n", boot_info->kernel_origin_base);
  }
}

void __attribute__((naked)) apu_boot() {
  // #if defined(__WIN32__)
  // asm("jmpl $0x0000, $_init_apu_boot\n");
  // #else
  // asm("jmpl $0x0000, $init_apu_boot\n");
  // #endif
}

void disk_init_floppy(char* fileName) {
  FILE* fp = fopen(fileName, "w");
  char buf[512];
  memset(buf, 0, 512);
  for (int cylinder = 0; cylinder < 80; cylinder++) {
    for (int head = 0; head < 2; head++) {
      for (int sector = 1; sector <= 18; sector++) {
        fwrite(buf, 512, 1, fp);
      }
    }
  }
  fclose(fp);
}

void disk_read_floppy(int cylinder, int head, int sector, FILE* fp, char* buf) {
  int index = 18 * 2 * 512 * cylinder + 18 * 512 * head + 512 * (sector - 1);
  int tmp = (int)ftell(fp);
  fseek(fp, index, SEEK_SET);
  fread(buf, 512, 1, fp);
  fseek(fp, tmp, SEEK_SET);
}

void disk_write_floppy(int cylinder, int head, int sector, FILE* fp,
                       char* buf) {
  int index = 18 * 2 * 512 * cylinder + 18 * 512 * head + 512 * (sector - 1);
  int tmp = (int)ftell(fp);
  fseek(fp, index, SEEK_SET);
  fwrite(buf, 512, 1, fp);
  fseek(fp, tmp, SEEK_SET);
}

u8 disk_read(u16 disk, u16 head, u16 cylinder, u16 sector, u16 number,
             u32* addr, u8* status) {
  u8 carry = 0;
  if (disk_file == NULL) {
    printf("not found kernel file\n");
    return -1;
  }
  disk_read_floppy(cylinder, head, sector, disk_file, addr);
  *status = 0;
  return carry;
}

u8 disk_read_lba(u32 lba, u32* addr, u8* status) {
  u32 cylinder = lba / boot_info->disk.spt / boot_info->disk.hpc;
  u32 head = (lba / boot_info->disk.spt) % boot_info->disk.hpc;
  u32 sector = (lba % boot_info->disk.spt) + 1;

  //   // todo read by disk type
  printf("addr:%x lba:%d CHS = (%d, %d,%d)\r\n", addr, lba, cylinder, head,
         sector);
  u8 ret = disk_read(0, head, cylinder, sector, 1, addr, status);
  if (ret == 0) {
    if (addr == 0x1010000) {
      printf("addr %x ", addr);
      print_hex(addr);
    }
  } else {
    printf("\n\rread erro on lba:%x cylinder:%d head:%d sector:%d\n\r", lba,
           cylinder, head, sector);
  }
  return ret;

  // u32 ret = -1;
  // FILE* fp = fopen("init/kernel.elf", "rb");
  // if (fp == NULL) {
  //   printf("not found kernel file\n");
  //   return -1;
  // }

  // printf("read from %x to addr : %x\n", lba, addr);

  // int n = fseek(fp, lba, SEEK_SET);
  // if (n < 0) {
  //   printf("seek error at %x\n", lba);
  // }
  // n = fread(addr, READ_BLOCK_SIZE, 1, fp);
  // // print_hex(addr);

  // if (n > 0) {
  //   ret = 0;
  // }
  // fclose(fp);
  // return ret;
}

void read_kernel() {
  //   u8 status = 0;
  //   print_string("read kernel\n");
  //   u32 cylinder = 0;
  //   u32 head = 0;
  //   u32 sector = 0xc;  // 0xc;
  //   u32 lba = 0x00;
  //   u8 ret = 0;
  //   if (boot_info->disk.type == 1) {
  //     lba = (cylinder * boot_info->disk.hpc + head) * boot_info->disk.spt +
  //           (sector);
  //     // lba=sector;
  //     // flopy 1.44m
  // #ifdef KERNEL_MOVE
  //     u32* addr = boot_info->kernel_origin_base;
  // #else
  //     u32* addr = boot_info->kernel_base;
  // #endif

  //     for (int i = 0; i < KERNEL_BLOCK_SIZE * 2; i++) {
  //       // disk sector number
  //       ret = disk_read_lba(lba, addr, &status);
  //       if (ret == 0) {
  //         print_string(".");
  //       } else {
  //         printf("\n\rread kernel offset %d error code %d addr:%x\n\r", i,
  //         status,
  //                addr);
  //         break;
  //       }
  //       lba++;
  //       addr += READ_BLOCK_SIZE;
  //     }
  //   }
  //   if (ret == 0) {
  //     print_string("\n\rread kernel success\n\r");
  //   } else {
  //     print_string("\n\rread kernel error\n\r");
  //     for (;;)
  //       ;
  //   }

  u32 read_size = (u32)boot_info->kernel_size * 4;
  boot_info->kernel_origin_base = malloc(read_size);
  FILE* fp = fopen("init/kernel.elf", "r+");
  if (fp != NULL) {
    fread(boot_info->kernel_origin_base, read_size, 1, fp);
    fclose(fp);
  }

  print_hex(boot_info->kernel_origin_base);

  // move apu entry
  // memmove(SECOND_BOOT_ENTRY, &apu_boot, 32 * 8);
  boot_info->second_boot_entry = SECOND_BOOT_ENTRY;
}

void print_hex(u32* addr) {
  for (int x = 0; x < 16; x++) {
    printf("addr %p: %08x\n", addr + x, addr[x]);
  }
  printf("\n\r");
}

void init_boot_info() {
  boot_info = &boot_data;
  boot_info->version = BOOT_VERSION;
  boot_info->kernel_origin_base = KERNEL_ORIGIN_BASE;
  boot_info->kernel_base = KERNEL_BASE;
  boot_info->kernel_size = KERNEL_BLOCK_SIZE * READ_BLOCK_SIZE * 2;
  boot_info->gdt_number = GDT_NUMBER;
  boot_info->pdt_base = PDT_BASE;
  boot_info->tss_number = CPU_NUMBER;
}

void init_disk() {
  boot_info->disk.hpc = 2;
  boot_info->disk.spt = 18;
  boot_info->disk.type = 1;  // 1.44m floppy flat lba

  disk_file = fopen("image/duck.img", "rb");
}

void init_display() {
  boot_info->disply.mode = 1;
  boot_info->disply.video = 0xB8000;
  boot_info->disply.height = 25;
  boot_info->disply.width = 80;
}

void init_boot() {
  init_boot_info();
  cls();
  printf("boot info addr %x\n\r", boot_info);

  init_gdt();

  print_string("init display\n\r");
  init_display();

  print_string("init memory\n\r");
  init_memory();

  init_disk();

  read_kernel();
  init_cpu();

  if (disk_file != NULL) {
    fclose(disk_file);
  }

  // asm volatile(
  //     "movl %0, %%esp\n"
  //     "mov %%esp,%%ebp\n"
  //     :
  //     : "m"(kernel_stack_top));

  start_kernel();
  // #if defined(__WIN32__)
  //   asm("jmpl %0, $_start_kernel" ::"i"(GDT_ENTRY_32BIT_CS * GDT_SIZE));
  // #else
  //   asm("jmpl %0, $start_kernel" ::"i"(GDT_ENTRY_32BIT_CS * GDT_SIZE));
  // #endif

  for (;;)
    ;
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
  printf("e_phnum:%d\n\r", elf_header->e_phnum);
  printf("e_shnum:%d\n\r", elf_header->e_shnum);

  void* elf = elf_header;
  Elf32_Phdr* phdr = (elf + elf_header->e_phoff);
  printf("addr %x elf=%x\n\r", phdr, elf);
  u32 entry = 0;
  for (int i = 0; i < elf_header->e_phnum; i++) {
    printf("type:%d\n\r", phdr[i].p_type);
    switch (phdr[i].p_type) {
      case PT_NULL:
        printf(" %s %x %x %x %s %x %x \r\n", "NULL", phdr[i].p_offset,
               phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
               phdr[i].p_memsz);
        break;
      case PT_LOAD: {
        if ((phdr[i].p_flags & PF_X) == PF_X) {
          printf(" %s %x %x %x %s %x %x \r\n", "LOAD", phdr[i].p_offset,
                 phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                 phdr[i].p_memsz);
          char* start = elf + phdr[i].p_offset;
          char* vaddr = phdr[i].p_vaddr;
          printf("load start:%p vaddr:%p size:%x %d \n\r", start, vaddr,
                 phdr[i].p_filesz, phdr[i].p_filesz);

          char buf[64];
          sprintf(buf, "txt%d_mmap", i);
          map_segment(buf, vaddr, phdr[i].p_filesz);

          entry = vaddr;

          // print_hex(start);
          memmove(vaddr, start, phdr[i].p_filesz);
          // memmove32(vaddr,start ,phdr[i].p_filesz);
          printf("move end\n\r");
        } else {
          printf(" %s %x %x %x %s %x %x \r\n", "LOAD", phdr[i].p_offset,
                 phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
                 phdr[i].p_memsz);
          char* start = elf + phdr[i].p_offset;
          char* vaddr = phdr[i].p_vaddr;
          printf("load start:%p vaddr:%p size:%x %d \n\r", start, vaddr,
                 phdr[i].p_filesz, phdr[i].p_filesz);
          char buf[64];
          sprintf(buf, "txt%d_mmap", i);
          map_segment(buf, vaddr, phdr[i].p_filesz);

          entry = vaddr;

          // print_hex(start);
          memmove(vaddr, start, phdr[i].p_filesz);
          // memmove32(vaddr,start ,phdr[i].p_filesz);
          printf("move2 end\n\r");
        }
        break;
      }
      default:
        print_segment(phdr, i);
        break;
    }
  }

  Elf32_Shdr* shdr = (elf + elf_header->e_shoff);
  for (int i = 0; i < elf_header->e_shnum; i++) {
    if (SHT_NOBITS == shdr[i].sh_type) {
      char* vaddr = shdr[i].sh_addr;
      char* start = shdr[i].sh_offset;

      printf("load shdr nobits start:%x vaddr:%x size:%x \n\r", start, vaddr,
             shdr[i].sh_size);

      char buf[64];
      sprintf(buf, "data%d_mmap", i);
      map_segment(buf, vaddr, shdr[i].sh_size);

      // memset(vaddr, 0, shdr[i].sh_size);
      //  map_alignment(page,vaddr,buf,shdr[i].sh_size);
    } else if (entry != shdr[i].sh_addr &&
               (shdr[i].sh_type & SHT_PROGBITS == SHT_PROGBITS) &&
               (shdr[i].sh_flags & SHF_ALLOC == SHF_ALLOC) &&
               (shdr[i].sh_flags & SHF_WRITE == SHF_WRITE)) {
      char* start = shdr[i].sh_offset;
      char* vaddr = shdr[i].sh_addr;
      printf("load shdr %s start:%x vaddr:%x size:%x \n\r", shdr[i].sh_name,
             start, vaddr, shdr[i].sh_size);
      u32* phstart = (u32)elf + shdr[i].sh_offset;

      char buf[64];
      sprintf(buf, "data%d_mmap", i);
      map_segment(buf, vaddr, shdr[i].sh_size);

      memmove(phstart, vaddr, shdr[i].sh_size);
    }
  }
}

void* load_kernel() {
#ifdef KERNEL_MOVE
  u32* elf = boot_info->kernel_origin_base;
#else
  u32* elf = boot_info->kernel_base;
#endif

  Elf32_Ehdr* elf_header = (Elf32_Ehdr*)elf;
  if (elf_header->e_ident[0] == ELFMAG0 || elf_header->e_ident[1] == ELFMAG1) {
    printf("header: ");
    for (int x = 0; x < 16; x++) {
      printf("%x ", elf_header->e_ident[x]);
    }
    printf("\n\r");

    printf("type:%d\n\r", elf_header->e_type);
    printf("e_machine:%d\n\r", elf_header->e_machine);
    printf("e_entry:%d\n\r", elf_header->e_entry);
    printf("e_phoff:%d\n\r", elf_header->e_phoff);
    printf("e_shoff:%d\n\r", elf_header->e_shoff);
    printf("e_ehsize:%d\n\r", elf_header->e_ehsize);
    printf("e_phentsize:%d\n\r", elf_header->e_phentsize);

    load_elf(elf_header);
    return elf_header->e_entry;
  } else {
    printf("bin kernel\n\r");
    return boot_info->kernel_base;
  }
}

void print_segment(Elf32_Phdr* phdr, int i) {
  switch (phdr[i].p_type) {
    case PT_DYNAMIC:
      printf(" %s %x %x %x\r\n %s %x %x ", "DYNAMIC", phdr[i].p_offset,
             phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
             phdr[i].p_memsz);
      break;
    case PT_INTERP:
      printf(" %s %x %x %x\r\n %s %x %x ", "INTERP", phdr[i].p_offset,
             phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
             phdr[i].p_memsz);
      break;
    case PT_NOTE:
      printf(" %s %x %x %x\r\n %s %x %x ", "NOTE", phdr[i].p_offset,
             phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
             phdr[i].p_memsz);
      break;
    case PT_SHLIB:
      printf(" %s %x %x %x\r\n %s %x %x ", "SHLIB", phdr[i].p_offset,
             phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
             phdr[i].p_memsz);
      break;
    case PT_PHDR:
      printf(" %s %x %x %x\r\n %s %x %x ", "PHDR", phdr[i].p_offset,
             phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
             phdr[i].p_memsz);
      break;
    case PT_TLS:
      printf(" %s %x %x %x\r\n %s %x %x ", "TLS", phdr[i].p_offset,
             phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
             phdr[i].p_memsz);
      break;
    case PT_NUM:
      printf(" %s %x %x %x\r\n %s %x %x ", "NUM", phdr[i].p_offset,
             phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
             phdr[i].p_memsz);
      break;
    case PT_GNU_EH_FRAME:
      printf(" %s %x %x %x\r\n %s %x %x ", "GNU_EH_FRAME", phdr[i].p_offset,
             phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
             phdr[i].p_memsz);
      break;
    case PT_GNU_RELRO:
      printf(" %s %x %x %x\r\n %s %x %x ", "GNU_RELRO", phdr[i].p_offset,
             phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
             phdr[i].p_memsz);
      break;
    case PT_GNU_STACK:
      printf(" %s %x %x %x\r\n %s %x %x ", "GNU_STACK", phdr[i].p_offset,
             phdr[i].p_vaddr, phdr[i].p_paddr, "", phdr[i].p_filesz,
             phdr[i].p_memsz);
      break;
    default:
      break;
  }
}

void start_kernel() {
  // asm volatile("movl %0, %%ss" : : "r"(GDT_ENTRY_32BIT_DS * GDT_SIZE));
  // asm volatile("movl %0, %%ds" : : "r"(GDT_ENTRY_32BIT_DS * GDT_SIZE));
  // asm volatile("movl %0, %%es" : : "r"(GDT_ENTRY_32BIT_DS * GDT_SIZE));
  // asm volatile("movl %0, %%gs" : : "r"(GDT_ENTRY_32BIT_DS * GDT_SIZE));
  // asm volatile("movl %0, %%fs" : : "r"(GDT_ENTRY_32BIT_FS * GDT_SIZE));

  print_string("load kernel\n\r");
  boot_info->kernel_entry = load_kernel();
  entry kmain = boot_info->kernel_entry;
  printf("====>load kernel at %p\n\r", kmain);
  print_hex(kmain);
  printf("====>start code end\n");

  int argc = 0;
  char** argv = 0;
  char* envp[10];
  envp[0] = boot_info;
  envp[1] = cpu_id++;
  printf("====>start kmain\n");

  kmain(argc, argv, envp);
}

int main(int argc, char* args[]) {
  printf("init duck\n");
  init_boot();

  // SDL_Window* window;
  // SDL_Renderer* renderer;
  // // Initialize SDL.
  // if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

  // // Create the window where we will draw.
  // window = SDL_CreateWindow("YiYiYa Dumulator", SDL_WINDOWPOS_CENTERED,
  //                           SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
  // SDL_assert(window);
  // // We must call SDL_CreateRenderer in order for draw calls to affect this
  // // window.
  // renderer = SDL_CreateRenderer(window, -1, 0);

  // // Select the color for drawing. It is set to red here.
  // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  // // Clear the entire screen to our selected color.
  // SDL_RenderClear(renderer);

  // // Up until now everything was drawn behind the scenes.
  // // This will show the new, red contents of the window.
  // SDL_RenderPresent(renderer);

  // SDL_StartTextInput();
  // // Give us time to see the window.
  // // SDL_Delay(5000);

  // int is_quit = 0;
  // while (!is_quit) {
  //   SDL_Event event;
  //   if (SDL_PollEvent(&event)) {
  //     printf("sdl event %d\n", event.type);
  //     switch (event.type) {
  //       case SDL_QUIT:
  //         is_quit = 1;
  //         break;
  //       case SDL_MOUSEMOTION:
  //         printf("SDL mouse motion: x=%d,y=%d\n", event.motion.x,
  //                event.motion.y);
  //         break;
  //       case SDL_MOUSEBUTTONDOWN:
  //         if (event.button.button == SDL_BUTTON_LEFT) {
  //           printf("SDL button left down\n");
  //           SDL_ShowCursor(false);
  //         } else if (event.button.button == SDL_BUTTON_MIDDLE) {
  //           printf("SDL button middle down\n");
  //         } else if (event.button.button == SDL_BUTTON_RIGHT) {
  //           printf("SDL button right down\n");
  //           SDL_ShowCursor(true);
  //         }
  //         break;
  //       case SDL_MOUSEBUTTONUP:
  //         if (event.button.button == SDL_BUTTON_LEFT) {
  //           printf("SDL button left up\n");
  //         } else if (event.button.button == SDL_BUTTON_MIDDLE) {
  //           printf("SDL button middle up\n");
  //         } else if (event.button.button == SDL_BUTTON_RIGHT) {
  //           printf("SDL button right up\n");
  //         }
  //         break;
  //       case SDL_MOUSEWHEEL:
  //         printf("SDL wheel dir:%d\n", event.wheel.direction);
  //         printf("SDL wheel x:%d, y:%d\n", event.wheel.x, event.wheel.y);
  //         break;
  //       case SDL_KEYDOWN:
  //         printf("SDL key down sym:%x(%s), mod:%x\n", event.key.keysym.sym,
  //                SDL_GetKeyName(event.key.keysym.sym),
  //                event.key.keysym.mod);
  //         break;
  //       case SDL_KEYUP:
  //         printf("SDL key up sym:%x(%s), mod:%x\n", event.key.keysym.sym,
  //                SDL_GetKeyName(event.key.keysym.sym),
  //                event.key.keysym.mod);
  //         break;
  //       case SDL_WINDOWEVENT:
  //         printf("SDL window event %d\n", event.window.event);
  //         break;
  //       case SDL_TEXTINPUT:
  //         printf("SDL text input %s\n", event.text.text);
  //         break;
  //       default:
  //         break;
  //     }
  //   }
  //   SDL_Delay(10);
  // }
  // printf("sdl exit\n");
  // SDL_Quit();
  return 0;
}