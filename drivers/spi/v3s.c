/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "gpio/v3s.h"

#include "kernel/kernel.h"
#include "spi.h"
#include "v3s-reg-ccu.h"

#define SPI0_BASE 0x01C68000

// #define kdbg

typedef struct v3s_spi {
  u32 reserved;   // reserved
  u32 gcr;        // SPI Global Control Register
  u32 tcr;        // SPI Transfer Control register
  u32 reserved0;  // reserved
  u32 ier;        // SPI Interrupt Control register
  u32 isr;        // SPI Interrupt Status register
  u32 fcr;        // SPI FIFO Control register
  u32 fsr;        // SPI FIFO Status register
  u32 wcr;        // SPI Wait Clock Counter register
  u32 ccr;        // SPI Clock Rate Control register
  u32 reserved1;  // reserved
  u32 reserved2;  // reserved
  u32 mbc;        // SPI Burst Counter register
  u32 mtc;        // SPI Transmit Counter Register
  u32 bcc;        // SPI Burst Control register
  u32 reserved3[(0x200 - 0x38) / 4];
  u32 txd;  // SPI TX Data register
  u32 rxd;  // SPI RX Data register
} v3s_spi_t;

// use SPI0_BASE
static v3s_spi_t* v3s_spi_base = SPI0_BASE;

void v3s_spi_cs(spi_t* spi, u32 val) {
  u32 r = v3s_spi_base->tcr;
  r &= ~((3 << 4) | (1 << 7));
  r |= (val & 3) << 4 | (0 << 7);
  v3s_spi_base->tcr = r;
}

u32 v3s_spi_rw_data(spi_t* spi, spi_msg_t* msg) {
  u32 ret = 0;
  u32 tx_len = msg->tx_len;
  u32 rx_len = msg->rx_len;

  u8* tx_buf = msg->tx_buf;
  u8* rx_buf = msg->rx_buf;
  u32 fifo_byte = 8;
  if (tx_len < fifo_byte) {
    fifo_byte = 1;
  }
  // kprintf("tx_len %d tx buf %x\n", tx_len, tx_buf);
  for (; tx_len > 0;) {
    // set Master Burst Counter
    v3s_spi_base->mtc = fifo_byte;
    // Master Write Transmit Counter
    v3s_spi_base->mbc = fifo_byte;
    v3s_spi_base->bcc = fifo_byte;

    if (tx_buf != NULL) {
      u32 len = fifo_byte;
      // fill data
      for (; len > 0; len--) {
        v3s_spi_base->txd = *tx_buf++;
      }
      tx_len -= fifo_byte;
    }

    // start trans
    v3s_spi_base->tcr |= 1 << 31;

    // kprintf("trans %x\n", v3s_spi_base->isr);
    // wait finish Transfer Completed
    // while ((v3s_spi_base->isr & (1 << 12)) == 0);
    while ((v3s_spi_base->fsr & 0xff) < fifo_byte)
      ;
    // kprintf("trans end %d %d\n", rx_len, tx_len);

    // clear flag
    // v3s_spi_base->isr = 1 << 12;

    if (rx_buf != NULL) {
      u32 len = fifo_byte;
      for (; len > 0; len--) {
        *rx_buf++ = v3s_spi_base->rxd;
      }
      rx_len -= fifo_byte;
    }
  }

  return msg->tx_len;
}


static void v3s_spi_write_txbuf(spi_t* spi, u8* buf, int len) {
  int i;
  v3s_spi_base->mtc = len & 0xffffff;
  v3s_spi_base->bcc = len & 0xffffff;
  if (buf) {
    for (i = 0; i < len; i++) v3s_spi_base->txd = *buf++;
  } else {
    for (i = 0; i < len; i++) v3s_spi_base->txd = 0xff;
  }
}

u32 v3s_spi_xfer(spi_t* spi, spi_msg_t* msg, u32 cnt) {
  u32 bits=640;
  u32 len=msg->tx_len;
  int count = len * bits / 8;
  u8* tx = msg->tx_buf;
  u8* rx = msg->rx_buf;
  u8 val;
  int n, i;

  while (count > 0) {
    n = (count <= 64) ? count : 64;
    v3s_spi_base->mbc = n;
    v3s_spi_write_txbuf(spi, tx, n);
    
    v3s_spi_base->tcr |=  (1 << 31);

    while (( v3s_spi_base->fsr & 0xff) < n)
      ;
    for (i = 0; i < n; i++) {
      val= v3s_spi_base->rxd ;
      if (rx) *rx++ = val;
    }

    if (tx) tx += n;
    count -= n;
  }
  return len;
}

u32 v3s_spi_read_write(spi_t* spi, u32* data, u32 count) {
  v3s_spi_xfer(spi, data, count);
  // if (data == NULL || count <= 0) {
  //   return 0;
  // }
  // spi_msg_t* msg = data;
  // if (spi->inited == 0) {
  //   spi->inited = 1;
  // }
  // u32 ret, i;
  // for (i = 0; i < count; i++, msg++) {
  //   if (msg->flags & SPI_READ) {
  //     ret = v3s_spi_rw_data(spi, msg);
  //   } else if (msg->flags & SPI_WRITE) {
  //     ret = v3s_spi_rw_data(spi, msg);
  //   } else {
  //     ret = v3s_spi_rw_data(spi, msg);
  //   }
  //   if (ret < 0) {
  //     kprintf("read write error %x\n", ret);
  //     break;
  //   }
  // }
  // if (spi->inited == 1) {
  //   spi->inited = 0;
  // }
  return count;
}


// POINT_COLOR
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40
#define BRRED 0XFC07
#define GRAY 0X8430
#define GRAY175 0XAD75
#define GRAY151 0X94B2
#define GRAY187 0XBDD7
#define GRAY240 0XF79E

spi_t* gspi = NULL;

void lcd_write_data(u8 data) {
  // cs -> dc
  // gpio_pull(GPIO_C, 2, GPIO_PULL_UP);
  // v3s_spi_cs(gspi, 1);
  lcd_set_dc(1);
  spi_msg_t msg;
  msg.tx_buf = &data;
  msg.tx_len = 1;
  v3s_spi_read_write(gspi, &msg, 1);
}

void lcd_write_data_word(u16 data) {
  // cs -> dc
  // gpio_pull(GPIO_C, 2, GPIO_PULL_UP);
  // v3s_spi_cs(gspi, 1);
  lcd_set_dc(1);
  spi_msg_t msg = {0};
  msg.tx_buf = &data;
  msg.tx_len = 2;
  v3s_spi_read_write(gspi, &msg, 1);
}

void lcd_write_cmd(u8 cmd) {
  // gpio_pull(GPIO_C, 2, GPIO_PULL_DOWN);
  // v3s_spi_cs(gspi, 0);
  lcd_set_dc(0);
  spi_msg_t msg = {0};
  msg.tx_buf = &cmd;
  msg.tx_len = 1;
  v3s_spi_read_write(gspi, &msg, 1);
}

void delay(void) {
  int i;
  for (i = 0; i < 100000; i++) {
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

void lcd_fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color) {
  u16 i, j;
  lcd_address_set(xsta, ysta, xend - 1, yend - 1);  //设置显示范围
  for (i = ysta; i < yend; i++) {
    for (j = xsta; j < xend; j++) {
      lcd_write_data(color);
    }
  }
}

void lcd_set_dc(u32 val) { gpio_output(GPIO_G, 0, val); }

void v3s_test_lcd(spi_t* spi) {
  gspi = spi;

  gpio_config(GPIO_G, 0, GPIO_OUTPUT);

  lcd_write_cmd(0x11);
  delay();

  // set horiztional
  lcd_write_cmd(0x36);
  lcd_write_data(0x00);

  //
  lcd_write_cmd(0x3A);
  lcd_write_data(0x05);

  lcd_write_cmd(0xB2);
  lcd_write_data(0x0C);
  lcd_write_data(0x0C);
  lcd_write_data(0x00);
  lcd_write_data(0x33);
  lcd_write_data(0x33);

  lcd_write_cmd(0xB7);
  lcd_write_data(0x35);

  lcd_write_cmd(0xBB);
  lcd_write_data(0x19);

  lcd_write_cmd(0xC0);
  lcd_write_data(0x2C);

  lcd_write_cmd(0xC2);
  lcd_write_data(0x01);

  lcd_write_cmd(0xC3);
  lcd_write_data(0x12);

  lcd_write_cmd(0xC4);
  lcd_write_data(0x20);

  lcd_write_cmd(0xC6);
  lcd_write_data(0x0F);

  lcd_write_cmd(0xD0);
  lcd_write_data(0xA4);
  lcd_write_data(0xA1);

  lcd_write_cmd(0xE0);
  lcd_write_data(0xD0);
  lcd_write_data(0x04);
  lcd_write_data(0x0D);
  lcd_write_data(0x11);
  lcd_write_data(0x13);
  lcd_write_data(0x2B);
  lcd_write_data(0x3F);
  lcd_write_data(0x54);
  lcd_write_data(0x4C);
  lcd_write_data(0x18);
  lcd_write_data(0x0D);
  lcd_write_data(0x0B);
  lcd_write_data(0x1F);
  lcd_write_data(0x23);

  lcd_write_cmd(0xE1);
  lcd_write_data(0xD0);
  lcd_write_data(0x04);
  lcd_write_data(0x0C);
  lcd_write_data(0x11);
  lcd_write_data(0x13);
  lcd_write_data(0x2C);
  lcd_write_data(0x3F);
  lcd_write_data(0x44);
  lcd_write_data(0x51);
  lcd_write_data(0x2F);
  lcd_write_data(0x1F);
  lcd_write_data(0x1F);
  lcd_write_data(0x20);
  lcd_write_data(0x23);
  lcd_write_cmd(0x21);

  lcd_write_cmd(0x29);

  kprintf("v3s test lcd end\n");
  lcd_fill(0, 0, 240, 240, RED);
}

int spi_init_device(device_t* dev) {
  spi_t* spi = kmalloc(sizeof(spi_t));
  dev->data = spi;

  spi->inited = 0;
  spi->read = v3s_spi_read_write;
  spi->write = v3s_spi_read_write;
  spi->cs = v3s_spi_cs;

  // map io spi0
  map_page(SPI0_BASE, SPI0_BASE, 0);

  u32 reg = 0;
  // set ahb clock gating
  reg = io_read32(V3S_CCU_BASE + CCU_BUS_CLK_GATE0);
  io_write32(V3S_CCU_BASE + CCU_BUS_CLK_GATE0, reg | 1 << 20);  // spi 0 gate

  // set spi0 sclk
  reg = io_read32(V3S_CCU_BASE + CCU_SPI0_CLK);
  io_write32(V3S_CCU_BASE + CCU_SPI0_CLK, reg | 1 << 31);

  // de assert spi
  reg = io_read32(V3S_CCU_BASE + CCU_BUS_SOFT_RST0);
  io_write32(V3S_CCU_BASE + CCU_BUS_SOFT_RST0, reg | 1 << 20);

  // gpio set miso pc0 SPI_MISO
  gpio_config(GPIO_C, 0, 3);  // 011: SPI0_MISO
  gpio_pull(GPIO_C, 0, GPIO_PULL_DISABLE);

  // gpio set sck pc1 SPI_SCK
  gpio_config(GPIO_C, 1, 3);  // 011: SPI0_CLK
  gpio_pull(GPIO_C, 1, GPIO_PULL_DISABLE);

  // gpio set cs pc2 SPI_CS
  gpio_config(GPIO_C, 2, 3);  // 011: SPI0_CS
  gpio_pull(GPIO_C, 2, GPIO_PULL_DISABLE);

  // gpio set mosi pc3 SPI_MOSI
  gpio_config(GPIO_C, 3, 3);  // 011: SPI0_MOSI
  gpio_pull(GPIO_C, 3, GPIO_PULL_DISABLE);

  // set master mode  stop transmit data when RXFIFO full
  v3s_spi_base->gcr = v3s_spi_base->gcr | 1 << 31 | 1 << 7 | 1 << 1 | 1;

  // wait
  while (v3s_spi_base->gcr & (1 << 31))
    ;
  kprintf("gcr %x\n", v3s_spi_base->gcr);

  // set SS Output Owner Select  1: Active low polarity (1 = Idle)
  v3s_spi_base->tcr = v3s_spi_base->tcr | 1 << 6 | 1 << 2;

  // clear intterrupt
  v3s_spi_base->isr = ~0;

  // set fcr TX FIFO Reset RX FIFO Reset
  v3s_spi_base->fcr = v3s_spi_base->fcr | 1 << 31 | 1 << 15;

  // set sclk clock  Select Clock Divide Rate 2 SPI_CLK = Source_CLK / (2*(n +
  // 1)).
  v3s_spi_base->ccr = 1 << 12 | 14;

  v3s_spi_cs(spi, 1);

  // v3s_test_lcd(spi);

  kprintf("v3s spi init end \n");

  return 0;
}
