#include "gpio.h"

void uart_send(unsigned int c) {
  unsigned int addr = 0x01c28000;
	while((io_read32(addr + 0x7c) & (0x1 << 1)) == 0);
	io_write32(addr + 0x00, c);
}

void platform_init(){
    
}