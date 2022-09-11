#include "arch/io.h"

#define PORT_COM1 0x3f8
#define PORT_COM2 0x2F8
#define PORT_COM3 0x3E8
#define PORT_COM4 0x2E8

int com_is_send() { return io_read8(PORT_COM1 + 5) & 0x20; }

int com_is_receive() { return io_read8(PORT_COM1 + 5) & 1; }

static int com_serial_init() {
  io_write8(PORT_COM1 + 1, 0x00);  // Disable all interrupts
  io_write8(PORT_COM1 + 3, 0x80);  // Enable DLAB (set baud rate divisor)
  io_write8(PORT_COM1 + 0, 0x03);  // Set divisor to 3 (lo byte) 38400 baud
  io_write8(PORT_COM1 + 1, 0x00);  //                  (hi byte)
  io_write8(PORT_COM1 + 3, 0x03);  // 8 bits, no parity, one stop bit
  io_write8(PORT_COM1 + 2,
            0xC7);  // Enable FIFO, clear them, with 14-byte threshold
  io_write8(PORT_COM1 + 4, 0x0B);  // IRQs enabled, RTS/DSR set
  io_write8(PORT_COM1 + 4, 0x1E);  // Set in loopback mode, test the serial chip
  io_write8(PORT_COM1 + 0, 0xAE);  // Test serial chip (send byte 0xAE and check
                                   // if serial returns same byte)

  if (io_read8(PORT_COM1 + 0) != 0xAE) {
    return 1;
  }

  io_write8(PORT_COM1 + 4, 0x0F);
  return 0;
}

void com_write(char a) {
  int i = 0;
  while (com_is_send() == 0) {
    i++;
  }
  if(i<=0){
    io_write8(PORT_COM1, a);
  }
}

char com_read() {
  int i=0;
  while (com_is_receive() == 0){
    i++;
  }
  return io_read8(PORT_COM1);
}

void platform_init() {
  com_serial_init();
  io_add_write_channel(com_write);
}

void platform_end(){
    
}


void timer_init(int hz){

}

void timer_end(){

}