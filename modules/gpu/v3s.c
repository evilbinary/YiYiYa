#include "v3s.h"

#include "vga/vga.h"
#include "v3s-reg-ccu.h"
#include "v3s-reg-de.h"
#include "v3s-reg-tcon.h"

#define RGB24_2_RGB565(r, g, b) \
  (u16)((((r) << 8) & 0xF800) | (((g) << 3) & 0x7E0) | (((b) >> 3)))

typedef struct pixel {
  u8 red;
  u8 green;
  u8 blue;
} pixel_t;

static void write_pixel(vga_device_t *vga, uint32_t x, uint32_t y,
                        const pixel_t *pix) {
  u32 color = RGB24_2_RGB565(pix->red, pix->green, pix->blue);
  vga->frambuffer[x + y * vga->width] = color;
}

static void inline v3s_de_enable(v3s_lcd_t *lcd) {
  struct de_glb_t *glb = (struct de_glb_t *)(lcd->de + V3S_DE_MUX_GLB);

  // map_page((u32)&glb->dbuff,(u32)&glb->dbuff,0);

  io_write32((u32)&glb->dbuff, 1);
}

static inline void v3s_de_set_address(v3s_lcd_t *lcd, void *vram) {
  struct de_ui_t *ui =
      (struct de_ui_t *)(lcd->de + V3S_DE_MUX_CHAN + 0x1000 * 2);
  // map_page((u32)&ui->cfg[0].top_laddr,(u32)&ui->cfg[0].top_laddr,0);

  io_write32((u32)&ui->cfg[0].top_laddr, (u32)vram);
}

static inline void v3s_de_set_mode(v3s_lcd_t *lcd) {
  struct de_clk_t *clk = (struct de_clk_t *)(lcd->de);
  struct de_glb_t *glb = (struct de_glb_t *)(lcd->de + V3S_DE_MUX_GLB);
  struct de_bld_t *bld = (struct de_bld_t *)(lcd->de + V3S_DE_MUX_BLD);
  struct de_ui_t *ui =
      (struct de_ui_t *)(lcd->de + V3S_DE_MUX_CHAN + 0x1000 * 2);
  u32 size = (((lcd->height - 1) << 16) | (lcd->width - 1));
  u32 val;
  int i;

  val = io_read32((u32)&clk->rst_cfg);
  val |= 1 << 0;
  io_write32((u32)&clk->rst_cfg, val);

  val = io_read32((u32)&clk->gate_cfg);
  val |= 1 << 0;
  io_write32((u32)&clk->gate_cfg, val);

  val = io_read32((u32)&clk->bus_cfg);
  val |= 1 << 0;
  io_write32((u32)&clk->bus_cfg, val);

  val = io_read32((u32)&clk->sel_cfg);
  val &= ~(1 << 0);
  io_write32((u32)&clk->sel_cfg, val);

  io_write32((u32)&glb->ctl, (1 << 0));
  io_write32((u32)&glb->status, 0);
  io_write32((u32)&glb->dbuff, 1);
  io_write32((u32)&glb->size, size);

  for (i = 0; i < 4; i++) {
    void *chan = (void *)(lcd->de + V3S_DE_MUX_CHAN + 0x1000 * i);
    kmemset(chan, 0, i == 0 ? sizeof(struct de_vi_t) : sizeof(struct de_ui_t));
  }
  kmemset(bld, 0, sizeof(struct de_bld_t));

  io_write32((u32)&bld->fcolor_ctl, 0x00000101);
  io_write32((u32)&bld->route, 2);
  io_write32((u32)&bld->premultiply, 0);
  io_write32((u32)&bld->bkcolor, 0xff000000);
  io_write32((u32)&bld->bld_mode[0], 0x03010301);
  io_write32((u32)&bld->bld_mode[1], 0x03010301);
  io_write32((u32)&bld->output_size, size);
  io_write32((u32)&bld->out_ctl, 0);
  io_write32((u32)&bld->ck_ctl, 0);
  for (i = 0; i < 4; i++) {
    io_write32((u32)&bld->attr[i].fcolor, 0xff000000);
    io_write32((u32)&bld->attr[i].insize, size);
  }

  io_write32(lcd->de + V3S_DE_MUX_VSU, 0);
  io_write32(lcd->de + V3S_DE_MUX_GSU1, 0);
  io_write32(lcd->de + V3S_DE_MUX_GSU2, 0);
  io_write32(lcd->de + V3S_DE_MUX_GSU3, 0);
  io_write32(lcd->de + V3S_DE_MUX_FCE, 0);
  io_write32(lcd->de + V3S_DE_MUX_BWS, 0);
  io_write32(lcd->de + V3S_DE_MUX_LTI, 0);
  io_write32(lcd->de + V3S_DE_MUX_PEAK, 0);
  io_write32(lcd->de + V3S_DE_MUX_ASE, 0);
  io_write32(lcd->de + V3S_DE_MUX_FCC, 0);
  io_write32(lcd->de + V3S_DE_MUX_DCSC, 0);

  io_write32((u32)&ui->cfg[0].attr,
             (1 << 0) | (4 << 8) | (1 << 1) | (0xff << 24));
  io_write32((u32)&ui->cfg[0].size, size);
  io_write32((u32)&ui->cfg[0].coord, 0);
  io_write32((u32)&ui->cfg[0].pitch, 4 * lcd->width);
  io_write32((u32)&ui->cfg[0].top_laddr, (u32)lcd->vram[lcd->index]);
  io_write32((u32)&ui->ovl_size, size);
}

static inline void v3s_tcon_enable(v3s_lcd_t *lcd) {
  struct v3s_tcon_reg_t *tcon = (struct v3s_tcon_reg_t *)lcd->tcon;
  u32 val;

  val = io_read32((u32)&tcon->ctrl);
  val |= (1 << 31);
  io_write32((u32)&tcon->ctrl, val);
}

static inline void v3s_tcon_disable(v3s_lcd_t *lcd) {
  struct v3s_tcon_reg_t *tcon = (struct v3s_tcon_reg_t *)lcd->tcon;
  u32 val;

  io_write32((u32)&tcon->ctrl, 0);
  io_write32((u32)&tcon->int0, 0);

  val = io_read32((u32)&tcon->tcon0_dclk);
  val &= ~(0xf << 28);
  io_write32((u32)&tcon->tcon0_dclk, val);

  io_write32((u32)&tcon->tcon0_io_tristate, 0xffffffff);
  io_write32((u32)&tcon->tcon1_io_tristate, 0xffffffff);
}

static inline void v3s_tcon_set_mode(v3s_lcd_t *lcd) {
  struct v3s_tcon_reg_t *tcon = (struct v3s_tcon_reg_t *)lcd->tcon;
  int bp, total;
  u32 val;

  val = io_read32((u32)&tcon->ctrl);
  val &= ~(0x1 << 0);
  io_write32((u32)&tcon->ctrl, val);

  val = (lcd->timing.v_front_porch + lcd->timing.v_back_porch +
         lcd->timing.v_sync_len) /
        2;
  io_write32((u32)&tcon->tcon0_ctrl, (1 << 31) | ((val & 0x1f) << 4));
  // val = clk_get_rate(lcd->clktcon) / lcd->timing.pixel_clock_hz;
  // val=0x01000000/lcd->timing.pixel_clock_hz;

  io_write32((u32)&tcon->tcon0_dclk, (0xf << 28) | ((val / 2) << 0));
  io_write32((u32)&tcon->tcon0_timing_active,
             ((lcd->width - 1) << 16) | ((lcd->height - 1) << 0));

  bp = lcd->timing.h_sync_len + lcd->timing.h_back_porch;
  total = lcd->width + lcd->timing.h_front_porch + bp;
  io_write32((u32)&tcon->tcon0_timing_h, ((total - 1) << 16) | ((bp - 1) << 0));
  bp = lcd->timing.v_sync_len + lcd->timing.v_back_porch;
  total = lcd->height + lcd->timing.v_front_porch + bp;
  io_write32((u32)&tcon->tcon0_timing_v, ((total * 2) << 16) | ((bp - 1) << 0));
  io_write32((u32)&tcon->tcon0_timing_sync,
             ((lcd->timing.h_sync_len - 1) << 16) |
                 ((lcd->timing.v_sync_len - 1) << 0));

  io_write32((u32)&tcon->tcon0_hv_intf, 0);
  io_write32((u32)&tcon->tcon0_cpu_intf, 0);

  if (lcd->bits_per_pixel == 18 || lcd->bits_per_pixel == 16) {
    io_write32((u32)&tcon->tcon0_frm_seed[0], 0x11111111);
    io_write32((u32)&tcon->tcon0_frm_seed[1], 0x11111111);
    io_write32((u32)&tcon->tcon0_frm_seed[2], 0x11111111);
    io_write32((u32)&tcon->tcon0_frm_seed[3], 0x11111111);
    io_write32((u32)&tcon->tcon0_frm_seed[4], 0x11111111);
    io_write32((u32)&tcon->tcon0_frm_seed[5], 0x11111111);
    io_write32((u32)&tcon->tcon0_frm_table[0], 0x01010000);
    io_write32((u32)&tcon->tcon0_frm_table[1], 0x15151111);
    io_write32((u32)&tcon->tcon0_frm_table[2], 0x57575555);
    io_write32((u32)&tcon->tcon0_frm_table[3], 0x7f7f7777);
    io_write32((u32)&tcon->tcon0_frm_ctrl, (lcd->bits_per_pixel == 18)
                                               ? ((1 << 31) | (0 << 4))
                                               : ((1 << 31) | (5 << 4)));
  }

  val = (1 << 28);
  if (!lcd->timing.h_sync_active) val |= (1 << 25);
  if (!lcd->timing.v_sync_active) val |= (1 << 24);
  if (!lcd->timing.den_active) val |= (1 << 27);
  if (!lcd->timing.clk_active) val |= (1 << 26);
  io_write32((u32)&tcon->tcon0_io_polarity, val);
  io_write32((u32)&tcon->tcon0_io_tristate, 0);
}

int v3s_lcd_init(vga_device_t *vga) {
  kprintf("v3s_lcd_init\n");
  v3s_lcd_t *lcd = kmalloc(sizeof(v3s_lcd_t));
  vga->priv = lcd;

  lcd->de = V3S_DE_BASE;
  lcd->tcon = V3S_TCON_BASE;

  lcd->rstde = 44;
  lcd->rsttcon = 36;
  lcd->width = vga->width;
  lcd->height = vga->height;
  lcd->bits_per_pixel = 16;
  lcd->bytes_per_pixel = 4;
  lcd->index = 0;
  lcd->vram[0] = vga->pframbuffer;
  lcd->vram[1] = vga->pframbuffer;
  vga->framebuffer_length = lcd->width * lcd->height * lcd->bytes_per_pixel * 8;

  lcd->timing.pixel_clock_hz = 33000000;
  lcd->timing.h_front_porch = 40;
  lcd->timing.h_back_porch = 87;
  lcd->timing.h_sync_len = 1;
  lcd->timing.v_front_porch = 13;
  lcd->timing.v_back_porch = 31;
  lcd->timing.v_sync_len = 1;
  lcd->timing.h_sync_active = 0;
  lcd->timing.v_sync_active = 0;
  lcd->timing.den_active = 1;
  lcd->timing.clk_active = 1;

  // map tcon 4k
  map_page(V3S_TCON_BASE, V3S_TCON_BASE, 0);
  // map ccu 1k
  map_page(V3S_CCU_BASE, V3S_CCU_BASE, 0);

  // map de 2m
  u32 addr = V3S_DE_BASE;
  for (int i = 0; i < 1024 * 1024 * 2 / PAGE_SIZE; i++) {
    map_page(addr, addr, 0);
    addr += 0x1000;
  }

  // vga->pframbuffer=kmalloc(vga->framebuffer_length*2);
  // map fb
  addr = vga->frambuffer;
  u32 paddr = vga->pframbuffer;
  for (int i = 0; i < vga->framebuffer_length / PAGE_SIZE; i++) {
    map_page(addr, paddr, PAGE_DEV);
    addr += 0x1000;
    paddr += 0x1000;
  }

  // init
  // todo clk video enable

  // todo gpio set
  // V3S_GPIOE0 set 3
  // V3S_GPIOE23 set 3

  // tcon init
  v3s_tcon_disable(lcd);
  v3s_de_set_mode(lcd);
  v3s_de_enable(lcd);
  v3s_tcon_set_mode(lcd);
  v3s_tcon_enable(lcd);

  v3s_de_set_address(lcd, lcd->vram[0]);
  v3s_de_enable(lcd);
  kprintf("v3s_lcd_init end\n");
}

void gpu_flush(vga_device_t *vga, u32 index) {
  vga->framebuffer_index = index;
  v3s_lcd_t *lcd = vga->priv;
  v3s_de_enable(lcd);
}

int gpu_init_mode(vga_device_t *vga, int mode) {
  if (mode == VGA_MODE_80x25) {
    vga->width = 80;
    vga->height = 25;
  } else if (mode == VGA_MODE_320x200x256) {
    vga->width = 320;
    vga->height = 256;
  } else if (mode == VGA_MODE_640x480x24) {
    vga->width = 640;
    vga->height = 480;
    vga->bpp = 24;
  } else if (mode == VGA_MODE_480x272x24) {
    vga->width = 480;
    vga->height = 272;
    vga->bpp = 24;
  } else if (mode == VGA_MODE_480x272x32) {
    vga->width = 480;
    vga->height = 272;
    vga->bpp = 32;
  } else if (mode == VGA_MODE_480x272x18) {
    vga->width = 480;
    vga->height = 272;
    vga->bpp = 18;
  } else if (mode == VGA_MODE_1024x768x32) {
    vga->width = 1024;
    vga->height = 768;
    vga->bpp = 32;
  } else {
    kprintf("no support mode %x\n");
  }
  vga->mode = mode;
  vga->write = NULL;
  // vga->flip_buffer=gpu_flush;

  vga->framebuffer_index = 0;
  vga->framebuffer_count = 1;
  vga->frambuffer = 0x73e00000;
  vga->pframbuffer = vga->frambuffer;

  v3s_lcd_init(vga);

  kprintf("fb addr:%x end:%x len:%x\n", vga->frambuffer,vga->frambuffer+vga->framebuffer_length, vga->framebuffer_length);


  // u8 *buffer = vga->frambuffer;
  // for (int i = 0; i < vga->framebuffer_length / 8; i++) {
  //   buffer[i] = 0x00;
  // }

  return 0;
}

// static void test_pixel(vga_device_t *vga) {
//   static const pixel_t BLACK = {0xff, 0x00, 0x00};
//   static const pixel_t RED = {0x00, 0x00, 0xff};

//   for (int i = 0; i < 300; i++) {
//     write_pixel(vga, i, 20, &BLACK);
//     write_pixel(vga, i, 40, &RED);
//     write_pixel(vga, i, 180, &BLACK);
//     write_pixel(vga, i, 190, &RED);
//   }
//   for (uint32_t j = 0; j < vga->height; j++) {
//     for (uint32_t i = 0; i < vga->width; i++) {
//       write_pixel(vga, i, j, &BLACK);
//     }
//   }
// }