#include "arch/io.h"

void uart_write(char a) {}

char uart_read() { return 0; }

void platform_init() { io_add_write_channel(uart_write); }

void platform_end() {}

void timer_init(int hz) {}

void timer_end() {}