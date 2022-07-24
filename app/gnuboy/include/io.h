#ifndef __IO_H__
#define __IO_H__

#include "defs.h"

#define SEND_ACTIVE 1
#define SEND_PASSIVE 0

struct io_handler {
	void (*init)();
	void (*send)(un8);
	void (*recv)();
	void (*shutdown)();
	void* data;
};

void io_handler_set(struct io_handler* hndlr);

void io_reset();

void io_init();
void io_send(un8);
void io_recv();
void io_shutdown();

#endif

