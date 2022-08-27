typedef void (*rom_write_char_uart_fn)(char c);
rom_write_char_uart_fn send=0x40007cf8;


void uart_send(unsigned int c) {
  send(c);
//   if (c == '\n') {
//     uart_send_ch(c);
//     c = '\r';
//   }
//   uart_send_ch(c);
}


void timer_init(int hz) {


}

void timer_end() {
  // kprintf("timer end %d\n",timer_count);
}

void platform_init(){
    // kprintf("platform_init\n");
    io_add_write_channel(uart_send);
}

void platform_end(){
 }