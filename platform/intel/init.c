
#define PORT_COM1 0x3f8
#define PORT_COM2	0x2F8
#define PORT_COM3	0x3E8
#define PORT_COM4	0x2E8

int com_is_send() { return io_read8(PORT_COM1 + 5) & 0x20; }

int com_is_receive() { return io_read8(PORT_COM1 + 5) & 1; }

void com_write(char a) {
  // while (is_send() == 0)
  //   ;
  io_write8(PORT_COM1, a);
}

char com_read() {
  while (is_receive() == 0)
    ;
  return io_read8(PORT_COM1);
}

void platform_init(){
    io_add_write_channel(com_write);
}