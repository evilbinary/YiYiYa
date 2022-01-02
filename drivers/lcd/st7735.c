/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "st7735.h"

#include "lcd.h"
#include "platform/stm32f4xx/gpio.h"

#define BLACK 0x0000
#define WHITE 0xFFFF

#define RED 0x001F
#define BLUE 0xF800
#define GREEN 0x00F
#define YELLOW 0x07FF
#define MAGENTA 0xF81F
#define CYAN 0xFFE0

device_t* spi_dev = NULL;

void delay(int n) {
  for (int i = 0; i < 10000 * n; i++) {
  }
}

void st7735_fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color) {
  u16 i, j;
  st7735_address_set(xsta, ysta, xend - 1, yend - 1);  //设置显示范围
  for (i = ysta; i < yend; i++) {
    for (j = xsta; j < xend; j++) {
      st7735_write_data_size(color, 2);
    }
  }
}

void st7735_set_pixel(u16 x, u16 y, u16 color) {
  st7735_address_set(x, y, x + 1, y + 1);
  st7735_write_data_size(color, 2);
}

void st7735_address_set(u16 x1, u16 y1, u16 x2, u16 y2) {
  st7735_write_cmd(0x2a);
  st7735_write_data(0x00);
  st7735_write_data(x1 + 2);
  st7735_write_data(0x00);
  st7735_write_data(x2 + 2);

  st7735_write_cmd(0x2b);
  st7735_write_data(0x00);
  st7735_write_data(y1 + 3);
  st7735_write_data(0x00);
  st7735_write_data(y2 + 3);

  st7735_write_cmd(0x2C);
}

void st7735_select() {
  gpio_output(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_RESET);
}

void st7735_unselect() {
  gpio_output(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_PIN_SET);
}

void st7735_reset() {
  gpio_output(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_PIN_RESET);
  st7735_select();
  delay(100);
  gpio_output(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_PIN_SET);
}

void st7735_write_cmd(u8 cmd) {
  gpio_output(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_RESET);
  st7735_select();
  spi_dev->write(spi_dev, &cmd, sizeof(cmd));
  st7735_unselect();
}

void st7735_write_data_size(u8 cmd, int size) {
  st7735_select();
  gpio_output(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_SET);
  spi_dev->write(spi_dev, &cmd, size);
  st7735_unselect();
}

void st7735_write_data(u8 cmd) {
  gpio_output(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_SET);
  st7735_select();
  spi_dev->write(spi_dev, &cmd, 1);
  st7735_unselect();
}

void st7735_init() {
  gpio_config(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_MODE_OUTPUT_PP);
  gpio_config(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_MODE_OUTPUT_PP);
  gpio_config(ST7735_CS_GPIO_Port, ST7735_CS_Pin, GPIO_MODE_OUTPUT_PP);
  delay(20);

  // init spi
  spi_dev = device_find(DEVICE_SPI);

  // init lcd
  st7735_reset();

  st7735_write_cmd(0x11);  // Sleep exit
  delay(40);

  // frmame rate
  st7735_write_cmd(0xB1);
  st7735_write_data(0x01);
  st7735_write_data(0x2c);
  st7735_write_data(0x2d);

  st7735_write_cmd(0xB2);
  st7735_write_data(0x01);
  st7735_write_data(0x2C);
  st7735_write_data(0x2D);

  st7735_write_cmd(0xB3);
  st7735_write_data(0x01);
  st7735_write_data(0x2C);
  st7735_write_data(0x2D);
  st7735_write_data(0x01);
  st7735_write_data(0x2C);
  st7735_write_data(0x2D);

  st7735_write_cmd(0xB4);  // Column inversion
  st7735_write_data(0x07);

  // ST7735R Power Sequence
  st7735_write_cmd(0xC0);
  st7735_write_data(0xA2);
  st7735_write_data(0x02);
  st7735_write_data(0x84);
  st7735_write_cmd(0xC1);
  st7735_write_data(0xC5);

  st7735_write_cmd(0xC2);
  st7735_write_data(0x0A);
  st7735_write_data(0x00);

  st7735_write_cmd(0xC3);
  st7735_write_data(0x8A);
  st7735_write_data(0x2A);
  st7735_write_cmd(0xC4);
  st7735_write_data(0x8A);
  st7735_write_data(0xEE);

  st7735_write_cmd(0xC5);  // VCOM
  st7735_write_data(0x0E);

  st7735_write_cmd(0x36);  // MX, MY, RGB mode
  st7735_write_data(0xC8);

  // ST7735R Gamma Sequence
  st7735_write_cmd(0xe0);
  st7735_write_data(0x0f);
  st7735_write_data(0x1a);
  st7735_write_data(0x0f);
  st7735_write_data(0x18);
  st7735_write_data(0x2f);
  st7735_write_data(0x28);
  st7735_write_data(0x20);
  st7735_write_data(0x22);
  st7735_write_data(0x1f);
  st7735_write_data(0x1b);
  st7735_write_data(0x23);
  st7735_write_data(0x37);
  st7735_write_data(0x00);
  st7735_write_data(0x07);
  st7735_write_data(0x02);
  st7735_write_data(0x10);

  st7735_write_cmd(0xe1);
  st7735_write_data(0x0f);
  st7735_write_data(0x1b);
  st7735_write_data(0x0f);
  st7735_write_data(0x17);
  st7735_write_data(0x33);
  st7735_write_data(0x2c);
  st7735_write_data(0x29);
  st7735_write_data(0x2e);
  st7735_write_data(0x30);
  st7735_write_data(0x30);
  st7735_write_data(0x39);
  st7735_write_data(0x3f);
  st7735_write_data(0x00);
  st7735_write_data(0x07);
  st7735_write_data(0x03);
  st7735_write_data(0x10);

  st7735_write_cmd(0x2a);
  st7735_write_data(0x00);
  st7735_write_data(0x00);
  st7735_write_data(0x00);
  st7735_write_data(0x7f);

  st7735_write_cmd(0x2b);
  st7735_write_data(0x00);
  st7735_write_data(0x00);
  st7735_write_data(0x00);
  st7735_write_data(0x9f);

  st7735_write_cmd(0xF0);  // Enable test command
  st7735_write_data(0x01);
  st7735_write_cmd(0xF6);  // Disable ram power save mode
  st7735_write_data(0x00);

  st7735_write_cmd(0x3A);   // 65k mode
  st7735_write_data(0x05);  // RGB 5-6-5-bit Input

  st7735_write_cmd(0x29);  // turn display on

  kprintf("lcd end\n");

  for(;;){
    
    st7735_fill(0, 0, 128, 128, RED);
  }
  // st7735_unselect();

}

int st7735_write_pixel(vga_device_t* vga, const void* buf, size_t len) {
  u16* color = buf;
  int i=0;
  for (i = 0; i < len / 6; i += 3) {
    st7735_set_pixel(color[i], color[i + 1], color[i + 2]);
  }
  return i;
}

int lcd_init_mode(vga_device_t* vga, int mode) {
  if (mode == VGA_MODE_128x128x16) {
    vga->width = 320;
    vga->height = 256;
  } else {
    kprintf("lcd no support mode %x\n");
  }
  vga->mode = mode;
  vga->write = st7735_write_pixel;
  // vga->flip_buffer=gpu_flush;

  vga->framebuffer_index = 0;
  vga->framebuffer_count = 1;
  vga->frambuffer = NULL;
  vga->pframbuffer = vga->frambuffer;

  st7735_init();

  return 0;
}