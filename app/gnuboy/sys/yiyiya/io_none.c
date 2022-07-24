#include "defs.h"
#include "hw.h"
#include "sys.h"
#include "io.h"
#include "regs.h"

void io_none_nop();
void io_none_send(un8 byte);

int io_setup_none(const char* link, struct io_handler* io_hdlr)
{
	io_hdlr->init = io_none_nop;
	io_hdlr->send = io_none_send;
	io_hdlr->recv = io_none_nop;
	io_hdlr->shutdown = io_none_nop;
	return link == 0;
}

void io_none_nop()
{
}

void io_none_send(un8 byte)
{
	/* FIXME - this is a hack for stupid roms that probe serial */
	R_SB = 0xff;
	hw_interrupt(IF_SERIAL, IF_SERIAL);
	hw_interrupt(0, IF_SERIAL);
}
