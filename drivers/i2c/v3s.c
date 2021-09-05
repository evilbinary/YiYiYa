/*******************************************************************
 * Copyright 2021-2080 evilbinary
 * 作者: evilbinary on 01/01/20
 * 邮箱: rootdebug@163.com
 ********************************************************************/
#include "i2c.h"
#include "kernel/kernel.h"

enum {
	I2C_STAT_BUS_ERROR	= 0x00,
	I2C_STAT_TX_START	= 0x08,
	I2C_STAT_TX_RSTART	= 0x10,
	I2C_STAT_TX_AW_ACK	= 0x18,
	I2C_STAT_TX_AW_NAK	= 0x20,
	I2C_STAT_TXD_ACK	= 0x28,
	I2C_STAT_TXD_NAK	= 0x30,
	I2C_STAT_LOST_ARB	= 0x38,
	I2C_STAT_TX_AR_ACK	= 0x40,
	I2C_STAT_TX_AR_NAK	= 0x48,
	I2C_STAT_RXD_ACK	= 0x50,
	I2C_STAT_RXD_NAK	= 0x58,
	I2C_STAT_IDLE		= 0xf8,
};


#define INT_FLAG (1 << 3)
#define M_STA (1 << 5)
#define M_STP (1 << 4)
#define A_ACK (1 << 2)

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
v3s_i2c_t* v3s_i2c_base = 0x01C2AC00;

u32 v3s_i2c_wait(u32 flag) {
  u32 timeout = 10000;
  for (; timeout > 0; timeout--) {
    if (v3s_i2c_base->cntr & flag) {
      return v3s_i2c_base->stat;
    }
  }
  return -1;
}

u32 v3s_i2c_start() {
  u32 val = v3s_i2c_base->cntr;
  val |= INT_FLAG | M_STA;  // Master Mode Stop
  v3s_i2c_base->cntr = val;
  v3s_i2c_wait(M_STA);
  return v3s_i2c_wait(INT_FLAG);
}

u32 v3s_i2c_stop() {
  u32 val = v3s_i2c_base->cntr;
  val |= M_STP | INT_FLAG;
  v3s_i2c_base->cntr = val;
  v3s_i2c_wait(M_STP);
  return v3s_i2c_wait(INT_FLAG);
}

u32 v3s_i2c_send_data(u32 data) {
  v3s_i2c_base->data = data;
  v3s_i2c_base->cntr = v3s_i2c_base->cntr | INT_FLAG;  // INT_FLAG
  return v3s_i2c_wait(INT_FLAG);
}

u32 v3s_i2c_recv_data() {
  u32 val = 0;
  u32 ret = v3s_i2c_send_data(val << 1 | 1);
  if (ret != I2C_STAT_TX_AR_ACK) {
    return -1;
  }
  v3s_i2c_base->cntr |= A_ACK;
  v3s_i2c_base->cntr |= ~A_ACK | INT_FLAG;
  v3s_i2c_wait(INT_FLAG);
  val = v3s_i2c_base->data;
  return val;
}

u32 v3s_i2c_read_data(i2c_t* i2c, u8* buf, u32 count) {
  if (i2c->inited == 0) {
    v3s_i2c_start();
    i2c->inited = 1;
  }
  u32 ret = v3s_i2c_send_data((u32)buf << 1 | 1);
  if (ret != I2C_STAT_TX_AR_ACK) {
    return -1;
  }
  v3s_i2c_base->cntr |= A_ACK;

  for (; count > 0; count--) {
    if (count == 1) {
      v3s_i2c_base->cntr |= ~A_ACK | INT_FLAG;
      v3s_i2c_wait(INT_FLAG);
      break;
    } else {
      v3s_i2c_base->cntr |= INT_FLAG;
      v3s_i2c_wait(INT_FLAG);
    }
    *buf++ = v3s_i2c_base->data;
  }
  return count;
}

u32 v3s_i2c_write_data(i2c_t* i2c, u8* buf, u32 count) {
  if (i2c->inited == 0) {
    v3s_i2c_start();
    i2c->inited = 1;
  }
  u32 ret = v3s_i2c_send_data((u32)buf << 1 | 1);
  if (ret != I2C_STAT_TX_AW_ACK) {
    return -1;
  }
  for (; count > 0; count--) {
    ret = v3s_i2c_send_data(*buf++);
    if (ret != I2C_STAT_TXD_ACK) {
      return -1;
    }
  }
  return count;
}

int i2c_init_device(device_t* dev) {
  i2c_t* i2c = kmalloc(sizeof(i2c_t));
  dev->data = i2c;

  i2c->inited = 0;
  i2c->read = v3s_i2c_read_data;
  i2c->write = v3s_i2c_write_data;

  return 0;
}
