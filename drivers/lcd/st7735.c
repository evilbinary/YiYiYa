/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "st7735.h"
#include "lcd.h"
#include "platform/stm32f4xx/gpio.h"

#define RED 0xF800

device_t *spi_dev = NULL;

void delay(int n) {
  for (int i = 0; i < 1000 * n; i++) {
  }
}

void lcd_fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color) {
  u16 i, j;
  lcd_address_set(xsta, ysta, xend - 1, yend - 1);  //设置显示范围
  for (i = ysta; i < yend; i++) {
    for (j = xsta; j < xend; j++) {
      lcd_write_data(color);
    }
  }
}

void lcd_address_set(u16 x1, u16 y1, u16 x2, u16 y2) {
  lcd_write_cmd(0x2a);
  lcd_write_data_word(x1);
  lcd_write_data_word(x2);

  lcd_write_cmd(0x2b);
  lcd_write_data_word(y1);
  lcd_write_data_word(y2);
  lcd_write_cmd(0x2C);
}

void st7735_select() {
  gpio_output(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_RESET);
}

void st7735_unselect() {
  gpio_output(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_SET);
}

void st7735_reset() {
  gpio_output(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_PIN_RESET);
  delay(5);
  gpio_output(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_PIN_SET);
}

void st7735_write_cmd(u8 cmd) {
  gpio_output(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_RESET);
  spi_dev->write(spi_dev, &cmd, sizeof(cmd));
}

void st7735_write_data(u8 cmd) {
  gpio_output(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_RESET);
  spi_dev->write(spi_dev, &cmd, sizeof(cmd));
}

void st7735_init() {
  gpio_config(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_MODE_OUTPUT_PP);
  gpio_config(ST7735_DC_GPIO_Port, ST7735_RES_Pin, GPIO_MODE_OUTPUT_PP);
  gpio_config(ST7735_DC_GPIO_Port, ST7735_CS_Pin, GPIO_MODE_OUTPUT_PP);

  st7735_select();
  st7735_reset();
  // init spi
  spi_dev = device_find(DEVICE_SPI);

  // init lcd
  // set horiztional
  st7735_write_cmd(0x36);
  st7735_write_data(0x00);

  //
  st7735_write_cmd(0x3A);
  st7735_write_data(0x05);

  st7735_write_cmd(0xB2);
  st7735_write_data(0x0C);
  st7735_write_data(0x0C);
  st7735_write_data(0x00);
  st7735_write_data(0x33);
  st7735_write_data(0x33);

  st7735_write_cmd(0xB7);
  st7735_write_data(0x35);

  st7735_write_cmd(0xBB);
  st7735_write_data(0x19);

  st7735_write_cmd(0xC0);
  st7735_write_data(0x2C);

  st7735_write_cmd(0xC2);
  st7735_write_data(0x01);

  st7735_write_cmd(0xC3);
  st7735_write_data(0x12);

  st7735_write_cmd(0xC4);
  st7735_write_data(0x20);

  st7735_write_cmd(0xC6);
  st7735_write_data(0x0F);

  st7735_write_cmd(0xD0);
  st7735_write_data(0xA4);
  st7735_write_data(0xA1);

  st7735_write_cmd(0xE0);
  st7735_write_data(0xD0);
  st7735_write_data(0x04);
  st7735_write_data(0x0D);
  st7735_write_data(0x11);
  st7735_write_data(0x13);
  st7735_write_data(0x2B);
  st7735_write_data(0x3F);
  st7735_write_data(0x54);
  st7735_write_data(0x4C);
  st7735_write_data(0x18);
  st7735_write_data(0x0D);
  st7735_write_data(0x0B);
  st7735_write_data(0x1F);
  st7735_write_data(0x23);

  st7735_write_cmd(0xE1);
  st7735_write_data(0xD0);
  st7735_write_data(0x04);
  st7735_write_data(0x0C);
  st7735_write_data(0x11);
  st7735_write_data(0x13);
  st7735_write_data(0x2C);
  st7735_write_data(0x3F);
  st7735_write_data(0x44);
  st7735_write_data(0x51);
  st7735_write_data(0x2F);
  st7735_write_data(0x1F);
  st7735_write_data(0x1F);
  st7735_write_data(0x20);
  st7735_write_data(0x23);
  st7735_write_cmd(0x21);

  st7735_write_cmd(0x29);

  kprintf("lcd end\n");
  for (;;) {
    lcd_fill(0, 0, 240, 240, RED);
  }

  st7735_unselect();
}



int lcd_init_mode(vga_device_t *vga, int mode) {
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
  vga->frambuffer = NULL;
  vga->pframbuffer = vga->frambuffer;

  st7735_init();

  return 0;
}