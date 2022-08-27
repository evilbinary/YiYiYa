#ifndef E1000_H
#define E1000_H

#include "kernel/kernel.h"

#define E1000_VENDOR_ID 0x8086
#define E1000_DEVICE_ID 0x100e

#define TX_BUF_SIZE 512
#define RX_BUF_SIZE 512

typedef struct e1000 {
  u32* mmio;

  char tx_buf[TX_BUF_SIZE];
  char rx_buf[RX_BUF_SIZE];


} e1000_t;

// 查手册Ethernet Controller Register Summary

#define TCTL 0x00400  // TCTL Transmit Control
#define TDBAL 0x03800  // Transmit Descriptor Base Low
#define TDBAH 0x03804  // Transmit Descriptor Base High
#define TDLEN 0x03808  // Transmit Descriptor Length
#define TDH 0x03810  // Transmit Descriptor Head R/W 317
#define TDT 0x03818  //  Transmit Descriptor Tail R/W 318

#define TIPG 0x00410  // Transmit IPG R/W 312


#define RCTL 0x00100  // Receive Control
#define RDBAL 0x02800  // Receive Descriptor Base Low
#define RDBAH 0x02804  // Receive Descriptor Base High
#define RDLEN 0x02808  // Receive Descriptor Length
#define RDH 0x02810  // Receive Descriptor Head R/W
#define RDT 0x02818  //  Receive Descriptor Tail R/W

#define DATA(addr) (*(volatile u32 *)(addr))

#endif