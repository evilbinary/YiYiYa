#ifndef __I2C_H__
#define __I2C_H__

#include "kernel/kernel.h"

#define 	I2C_READ 1
#define 	I2C_WRITE 2

typedef struct i2c i2c_t;

typedef size_t (*i2c_read_data_fn)(i2c_t* i2c, u32* data, size_t len);
typedef size_t (*i2c_write_data_fn)(i2c_t* i2c, u32* data, size_t len);

typedef struct i2c {
  u8 inited;
  i2c_read_data_fn read;
  i2c_write_data_fn write;
} i2c_t;


typedef struct i2c_msg {
	int addr;
	int flags;
	int len;
	u8* buf;
}i2c_msg_t;


u32 i2c_read_data(i2c_t* i2c, u8* buf, u32 count);

u32 i2c_write_data(i2c_t* i2c, u8* buf, u32 count);

#endif