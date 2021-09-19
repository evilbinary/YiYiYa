/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "gpio/v3s.h"

#include "i2c.h"
#include "kernel/kernel.h"
#include "v3s-reg-ccu.h"

#define TWI0_BASE 0x01C2AC00
#define TWI1_BASE 0x01C2B000
#define TWI2_BASE 0x01C2B400

#define I2C_STAT_BUS_ERROR 0x00
#define I2C_STAT_TX_START 0x08
#define I2C_STAT_TX_RSTART 0x10
#define I2C_STAT_TX_AW_ACK 0x18
#define I2C_STAT_TX_AW_NAK 0x20
#define I2C_STAT_TXD_ACK 0x28
#define I2C_STAT_TXD_NAK 0x30
#define I2C_STAT_LOST_ARB 0x38
#define I2C_STAT_TX_AR_ACK 0x40
#define I2C_STAT_TX_AR_NAK 0x48
#define I2C_STAT_RXD_ACK 0x50
#define I2C_STAT_RXD_NAK 0x58
#define I2C_STAT_IDLE 0xf8

#define INT_FLAG (1 << 3)
#define M_STA (1 << 5)
#define M_STP (1 << 4)
#define A_ACK (1 << 2)
#define BUS_EN (1 << 6)

#define kdbg

typedef struct v3s_i2c {
  u32 addr;   // TWI Slave address
  u32 xaddr;  // TWI Extended slave address
  u32 data;   // TWI Data byte
  u32 cntr;   // TWI Control register
  u32 stat;   // TWI Status register
  u32 ccr;    // TWI Clock control register
  u32 srst;   // TWI Software reset
  u32 efr;    // TWI Enhance Feature register
  u32 lcr;    // TWI Line Control register
} v3s_i2c_t;

// use TWI0 0x01C2AC00
static v3s_i2c_t* v3s_i2c_base = TWI0_BASE;

u32 v3s_i2c_wait(u32 flag) {
  u32 timeout = 2000;
  for (; timeout > 0; timeout--) {
    u32 cntr = v3s_i2c_base->cntr;
    // kprintf("wait cntr %x flag %x\n",cntr,flag);
    if (cntr & flag) {
      // kprintf("wait ok\n");
      return v3s_i2c_base->stat;
    }
  }
  // kprintf("wait long %x\n", v3s_i2c_base->stat);
  return -1;
}

u32 v3s_i2c_start() {
  u32 ret;
  u32 val = v3s_i2c_base->cntr;
  val |= INT_FLAG | M_STA;  // Master Mode Stop
  v3s_i2c_base->cntr = val;
  ret = v3s_i2c_wait(M_STA);
  kdbg;
  ret = v3s_i2c_wait(INT_FLAG);
  // kprintf("wait start %x\n", ret);
  return ret;
}

u32 v3s_i2c_stop() {
  u32 ret;
  u32 val = v3s_i2c_base->cntr;
  val |= M_STP | INT_FLAG;
  v3s_i2c_base->cntr = val;
  kdbg;
  ret = v3s_i2c_wait(M_STP);
  // kprintf("wait stop %x\n", ret);
  kdbg;
  return v3s_i2c_wait(INT_FLAG);
}

u32 v3s_i2c_send_data(u32 data) {
  v3s_i2c_base->data = data;
  v3s_i2c_base->cntr = v3s_i2c_base->cntr | INT_FLAG;  // INT_FLAG
  kdbg;
  return v3s_i2c_wait(INT_FLAG);
}

u32 v3s_i2c_recv_data() {
  u32 val = 0;
  u32 ret = v3s_i2c_send_data(val);
  if (ret != I2C_STAT_TX_AR_ACK) {
    return -1;
  }
  v3s_i2c_base->cntr |= A_ACK;
  v3s_i2c_base->cntr |= ~A_ACK | INT_FLAG;
  kdbg;
  v3s_i2c_wait(INT_FLAG);
  val = v3s_i2c_base->data;
  return val;
}

u32 v3s_i2c_read_data(i2c_t* i2c, i2c_msg_t* msg) {
  u32 len = msg->len;
  u8* buf = msg->buf;
  u8 addr = msg->addr;

  u32 ret = v3s_i2c_send_data((u8)(addr << 1 | 1));
  if (ret != I2C_STAT_TX_AR_ACK) {
    kprintf("send data error %x\n", ret);
    return -1;
  }
  v3s_i2c_base->cntr |= A_ACK;
  u32 i;
  for (i = 0; len > 0; len--) {
    if (len == 1) {
      v3s_i2c_base->cntr = (v3s_i2c_base->cntr & ~A_ACK) | INT_FLAG;
      kdbg;
      ret = v3s_i2c_wait(INT_FLAG);
      if (ret != I2C_STAT_RXD_NAK) {
        kprintf("rxd ack nak error %x\n", ret);
        return -1;
      }
      break;
    } else {
      v3s_i2c_base->cntr |= INT_FLAG;
      kdbg;
      ret = v3s_i2c_wait(INT_FLAG);
      if (ret != I2C_STAT_RXD_ACK) {
        kprintf("rxd ack error %x\n", ret);
        return -1;
      }
    }
    buf[i] = v3s_i2c_base->data;
    // kprintf("buf %x\n", (u32)buf[i]);
    i++;
  }
  return msg->len;
}

u32 v3s_i2c_write_data(i2c_t* i2c, i2c_msg_t* msg) {
  u32 ret = 0;
  u32 len = msg->len;
  u8* buf = msg->buf;
  ret = v3s_i2c_send_data((u8)(msg->addr << 1));
  if (ret != I2C_STAT_TX_AW_ACK) {
    kprintf(" send data error %x\n", ret);
    return -1;
  }
  for (; len > 0; len--) {
    ret = v3s_i2c_send_data(*buf++);
    if (ret != I2C_STAT_TXD_ACK) {
      kprintf(" send data error %x\n", ret);
      return -1;
    }
  }
  return msg->len;
}

u32 v3s_i2c_read_write(i2c_t* i2c, u32* data, u32 count) {
  if (data == NULL || count <= 0) {
    return 0;
  }
  i2c_msg_t* pmsg = data;
  if (i2c->inited == 0) {
    u32 ret = v3s_i2c_start();
    if (ret != I2C_STAT_TX_START) {
      kprintf("start error\n");
      return -1;
    }
    i2c->inited = 1;
  }
  u32 ret, i;
  for (i = 0; i < count; i++, pmsg++) {
    if (i != 0) {
      if (v3s_i2c_start() != I2C_STAT_TX_RSTART) {
        kprintf("start2 error\n");
        break;
      }
    }
    if (pmsg->flags & I2C_READ) {
      ret = v3s_i2c_read_data(i2c, pmsg);
    } else if (pmsg->flags & I2C_WRITE) {
      ret = v3s_i2c_write_data(i2c, pmsg);
    } else {
      ret = v3s_i2c_write_data(i2c, pmsg);
    }
    if (ret < 0) {
      kprintf("read write error %x\n", ret);
      break;
    }
  }
  if (i2c->inited == 1) {
    v3s_i2c_stop();
    i2c->inited = 0;
  }
  return count;
}

int i2c_init_device(device_t* dev) {
  i2c_t* i2c = kmalloc(sizeof(i2c_t));
  dev->data = i2c;

  i2c->inited = 0;
  i2c->read = v3s_i2c_read_write;
  i2c->write = v3s_i2c_read_write;

  // map io twi
  map_page(0x01C2A000, 0x01C2A000, 0);

  // De-assert twi
  u32 reg = io_read32(V3S_CCU_BASE + CCU_BUS_SOFT_RST4);
  io_write32(V3S_CCU_BASE + CCU_BUS_SOFT_RST4, reg | 1);

  // gpio set sda  pb7 TWI0_SDA
  gpio_config(GPIO_B, 7, 2);  // 010: TWI0_SDA
  gpio_pull(GPIO_B, 7, GPIO_PULL_UP);

  // gpio set scl pb6 TWI0_SCK
  gpio_config(GPIO_B, 6, 2);  // 010: TWI0_SCK
  gpio_pull(GPIO_B, 6, GPIO_PULL_UP);

  // Soft Reset TWI
  v3s_i2c_base->srst = 1 << 0;
  v3s_i2c_base->srst = 0 << 0;

  // set clock  CLK_N = 2, CLK_M=2 400kHz
  // set clock 100Khz, N=1, M=1, APB1 sourced from 24Mhz OSC
  u32 m = 2;
  u32 n = 2;
  v3s_i2c_base->ccr = ((m & 0xf) << 3) | ((n & 0x7) << 0);

  // clear slave addr
  v3s_i2c_base->addr = 0;
  v3s_i2c_base->xaddr = 0;

  // TWI Bus Enable
  v3s_i2c_base->cntr = BUS_EN | M_STP;

  return 0;
}
