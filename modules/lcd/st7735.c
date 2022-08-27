/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "st7735.h"

#include "lcd.h"
#include "platform/stm32f4xx/gpio.h"

#define USE_BUFF 1

#define BLACK 0x0000
#define WHITE 0xFFFF

#define RED 0xf800
#define BLUE 0x001f
#define GREEN 0x07e0
#define YELLOW 0xffe0
#define MAGENTA 0xF81F
#define CYAN 0xFFE0

device_t* spi_dev = NULL;

#ifdef USE_BUFF
#define ST77XX_BUF_SIZE 512
static uint8_t st7735_buf[ST77XX_BUF_SIZE];
static uint16_t st7735_buf_index = 0;

static void st77xx_write_buffer(u8* buff, size_t buff_size) {
  while (buff_size--) {
    st7735_buf[st7735_buf_index++] = *buff++;
    if (st7735_buf_index == ST77XX_BUF_SIZE) {
      spi_dev->write(spi_dev, st7735_buf, st7735_buf_index);
      st7735_buf_index = 0;
    }
  }
}

static void st77xx_flush_buffer(void) {
  if (st7735_buf_index > 0) {
    spi_dev->write(spi_dev, st7735_buf, st7735_buf_index);
    st7735_buf_index = 0;
  }
}
#endif

void delay(int n) {
  for (int i = 0; i < 10000 * n; i++) {
  }
}

void st7735_fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color) {
  u16 i, j;
  st7735_address_set(xsta, ysta, xend - 1, yend - 1);  //设置显示范围
  gpio_output(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_SET);
  for (i = ysta; i < yend; i++) {
    for (j = xsta; j < xend; j++) {
// st7735_write_data_size(color, 2);
#ifdef USE_BUFF
      st77xx_write_buffer((uint8_t*)&color, 2);
#else
      spi_dev->write(spi_dev, &color, 2);
#endif
    }
  }
#ifdef USE_BUFF
  st77xx_flush_buffer();
#endif
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
  delay(200);
  gpio_output(ST7735_RES_GPIO_Port, ST7735_RES_Pin, GPIO_PIN_SET);
  delay(200);
}

void st7735_write_cmd(u8 cmd) {
  gpio_output(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_RESET);
  spi_dev->write(spi_dev, &cmd, sizeof(cmd));
  // st7735_unselect();
}

void st7735_write_data_size(u8 cmd, int size) {
  gpio_output(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_SET);
  spi_dev->write(spi_dev, &cmd, size);
  // st7735_unselect();
}

void st7735_write_data(u8 cmd) {
  gpio_output(ST7735_DC_GPIO_Port, ST7735_DC_Pin, GPIO_PIN_SET);
  spi_dev->write(spi_dev, &cmd, 1);
  // st7735_unselect();
}

void st7735_test() {
    st7735_fill(0, 0, 128, 128, BLUE);
    st7735_fill(0, 0, 128, 128, GREEN);
    st7735_fill(0, 0, 128, 128, RED);
    kprintf("st7735 test lcd end\n");
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

  st7735_write_cmd(0xB1);  //帧率
  st7735_write_data(0x05);
  st7735_write_data(0x3C);
  st7735_write_data(0x3C);

  st7735_write_cmd(0xB2);  //帧率
  st7735_write_data(0x05);
  st7735_write_data(0x3C);
  st7735_write_data(0x3C);

  st7735_write_cmd(0xB3);  //帧率
  st7735_write_data(0x05);
  st7735_write_data(0x3C);
  st7735_write_data(0x3C);
  st7735_write_data(0x05);
  st7735_write_data(0x3C);
  st7735_write_data(0x3C);

  st7735_write_cmd(0xB4);  // Column inversion
  st7735_write_data(0x07);

  st7735_write_cmd(0x36);  // MX, MY, RGB mode
  st7735_write_data(0xC8);

  st7735_write_cmd(0x3A);   // 65k mode
  st7735_write_data(0x05);  // RGB 5-6-5-bit Input

  st7735_write_cmd(0x29);  // turn display on

  kprintf("st7735 lcd end\n");

  st7735_fill(0, 0, 128, 128, BLACK);
  // st7735_test();
}

int st7735_write_pixel(vga_device_t* vga, const void* buf, size_t len) {
  u16* color = buf;
  int i = 0;
  for (i = 0; i < len / 6; i += 3) {
    st7735_set_pixel(color[i], color[i + 1], color[i + 2]);
  }
  return i;
}

int lcd_init_mode(vga_device_t* vga, int mode) {
  if (mode == VGA_MODE_128x128x16) {
    vga->width = 128;
    vga->height = 128;
    vga->bpp=16;
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