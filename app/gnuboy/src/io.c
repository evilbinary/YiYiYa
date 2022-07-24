#include <stdio.h>

#include "io.h"
#include "debug.h"


struct io_handler* io_hdlr = NULL;

void io_handler_set(struct io_handler* hndlr)
{
	io_hdlr = hndlr;
}

void io_init()
{
	if (io_hdlr != NULL)
	{
		io_hdlr->init();
	}
}

void io_send(un8 byte)
{
	if (io_hdlr != NULL)
	{
		io_hdlr->send(byte);
	}
}

void io_recv()
{
	if (io_hdlr != NULL)
	{
		io_hdlr->recv();
	}
}

void io_shutdown()
{
	if (io_hdlr != NULL)
	{
		io_hdlr->shutdown();
	}
}

void io_reset()
{
	io_shutdown();
	io_init();
}
