#ifndef __SPI_H__
#define __SPI_H__

#include "kernel/kernel.h"

#define 	SPI_READ 1
#define 	SPI_WRITE 2

typedef struct spi spi_t;

typedef size_t (*spi_read_data_fn)(spi_t* spi, u32* data, size_t len);
typedef size_t (*spi_write_data_fn)(spi_t* spi, u32* data, size_t len);
typedef void (*spi_cs_fn)(spi_t* spi, u32 val);

typedef struct spi {
  u8 inited;
  spi_read_data_fn read;
  spi_write_data_fn write;
  spi_cs_fn cs;
  void* data;
} spi_t;


typedef struct spi_msg {
	int flags;
	u32 rx_len;
	u32 tx_len;
	u8* tx_buf;
	u8* rx_buf;
}spi_msg_t;


u32 spi_read_data(spi_t* spi, u8* buf, u32 count);

u32 spi_write_data(spi_t* spi, u8* buf, u32 count);

#endif