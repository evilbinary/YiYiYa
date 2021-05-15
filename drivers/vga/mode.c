/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "vga.h"

#define VGA_AC_INDEX 0x3C0
#define VGA_AC_WRITE 0x3C0
#define VGA_AC_READ 0x3C1
#define VGA_MISC_WRITE 0x3C2
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA 0x3C5
#define VGA_DAC_READ_INDEX 0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA 0x3C9
#define VGA_MISC_READ 0x3CC
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_DATA 0x3CF

#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA 0x3D5
#define VGA_INSTAT_READ 0x3DA

#define VGA_NUM_SEQ_REGS 5
#define VGA_NUM_CRTC_REGS 25
#define VGA_NUM_GC_REGS 9
#define VGA_NUM_AC_REGS 21
#define VGA_NUM_REGS \
  (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)

unsigned char g_80x25_text[] = {
    /* MISC */
    0x67,
    /* SEQ */
    0x03, 0x00, 0x03, 0x00, 0x02,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F, 0x00, 0x4F, 0x0D, 0x0E,
    0x00, 0x00, 0x00, 0x50, 0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00, 0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x3E, 0x3F, 0x0C, 0x00, 0x0F, 0x08, 0x00};

unsigned char g_320x200x256[] = {
    /* MISC
     *
     * 0x63 => 01100011
     * 7 6 5 4 3 2 1 0
     * 1 1 0 0 0 1 1 0
     * VSP HSP - - CS CS ERAM IOS
     * 7,6 - 480 lines
     * 5,4 - free
     * 3,2 - 28,322 MHZ Clock
     * 1 - Enable Ram
     * 0 - Map 0x3d4 to 0x3b4
     */
    0x63,
    /* SEQ */
    /**
     * index 0x00 - Reset
     * 0x03 = 11
     * Bits 1,0 Synchronous reset
     */
    0x03,
    /**
     * index 0x01
     * Clocking mode register
     * 8/9 Dot Clocks
     */
    0x01,
    /**
     * Map Mask Register, 0x02
     * 0x0F = 1111
     * Enable all 4 Maps Bits 0-3
     * chain 4 mode
     */
    0x0F,
    /**
     * map select register, 0x03
     * no character map enabled
     */
    0x00,
    /**
     * memory mode register 0x04
     * enables ch4,odd/even,extended memory
     */
    0x0E,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00};

unsigned char g_320x200x256_modex[] = {
    /* MISC */
    0x63,
    /* SEQ */
    0x03, 0x01, 0x0F, 0x00, 0x06,
    /* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E, 0x8F, 0x28, 0x00, 0x96, 0xB9, 0xE3,
    0xFF,
    /* GC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    /* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F, 0x41, 0x00, 0x0F, 0x00, 0x00};

void write_regs(unsigned char* regs) {
  unsigned i;

  /* write MISCELLANEOUS reg */
  io_write8(VGA_MISC_WRITE, *regs);
  regs++;
  /* write SEQUENCER regs */
  for (i = 0; i < VGA_NUM_SEQ_REGS; i++) {
    io_write8(VGA_SEQ_INDEX, i);
    io_write8(VGA_SEQ_DATA, *regs);
    regs++;
  }
  /* unlock CRTC registers */
  io_write8(VGA_CRTC_INDEX, 0x03);
  io_write8(VGA_CRTC_DATA, io_read8(VGA_CRTC_DATA) | 0x80);
  io_write8(VGA_CRTC_INDEX, 0x11);
  io_write8(VGA_CRTC_DATA, io_read8(VGA_CRTC_DATA) & ~0x80);
  /* make sure they remain unlocked */
  regs[0x03] |= 0x80;
  regs[0x11] &= ~0x80;
  /* write CRTC regs */
  for (i = 0; i < VGA_NUM_CRTC_REGS; i++) {
    io_write8(VGA_CRTC_INDEX, i);
    io_write8(VGA_CRTC_DATA, *regs);
    regs++;
  }
  /* write GRAPHICS CONTROLLER regs */
  for (i = 0; i < VGA_NUM_GC_REGS; i++) {
    io_write8(VGA_GC_INDEX, i);
    io_write8(VGA_GC_DATA, *regs);
    regs++;
  }
  /* write ATTRIBUTE CONTROLLER regs */
  for (i = 0; i < VGA_NUM_AC_REGS; i++) {
    (void)io_read8(VGA_INSTAT_READ);
    io_write8(VGA_AC_INDEX, i);
    io_write8(VGA_AC_WRITE, *regs);
    regs++;
  }
  /* lock 16-color palette and unblank display */
  (void)io_read8(VGA_INSTAT_READ);
  io_write8(VGA_AC_INDEX, 0x20);
}

static void write_pixel8(vga_device_t* vga, u32 x, u32 y, u32 c) {
  ((char*)vga->frambuffer)[vga->width * y + x] = c;
}

void vga_clear_screen(vga_device_t* vga) {
  u32 x = 0;
  u32 y = 0;
  for (y = 0; y < vga->height; y++) {
    for (x = 0; x < vga->width; x++) {
      ((char*)vga->frambuffer)[vga->width * y + x] = 0;
    }
  }
}

void vga_init_mode(vga_device_t* vga, int mode) {
  if (mode == VGA_MODE_80x25) {
    write_regs(g_80x25_text);
    vga->width = 80;
    vga->height = 25;
  } else if (mode == VGA_MODE_320x200x256) {
    write_regs(g_320x200x256);
    vga->width = 320;
    vga->height = 256;
  }
  vga->mode = mode;
  vga->write=write_pixel8;
  vga_clear_screen(vga);
}