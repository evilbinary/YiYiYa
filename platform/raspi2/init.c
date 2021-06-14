#include "gpio.h"

void uart_send(unsigned int c) {
  while(io_read32(UART0_FR)&0x20){}
  io_write32(UART0_DR,c);
}

void platform_init(){
    
}
