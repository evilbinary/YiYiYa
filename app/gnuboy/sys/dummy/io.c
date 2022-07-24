#include "defs.h"
#include "hw.h"
#include "sys.h"
#include "io.h"
#include "regs.h"

int io_dummy_setup(const char* link, struct io_handler* io_hdlr);
void io_dummy_nop();
void io_dummy_send(un8 byte);

struct io_handler dummy_io_hdlr;

int io_setup(const char* data){
	io_dummy_setup(data, &dummy_io_hdlr);
	io_handler_set(&dummy_io_hdlr);
	return 0;
}

int io_dummy_setup(const char* link, struct io_handler* io_hdlr)
{
	io_hdlr->init = io_dummy_nop;
	io_hdlr->send = io_dummy_send;
	io_hdlr->recv = io_dummy_nop;
	io_hdlr->shutdown = io_dummy_nop;
	return 1;
}

void io_dummy_nop()
{
}

void io_dummy_send(un8 byte)
{
	/* FIXME - this is a hack for stupid roms that probe serial */
	R_SB = 0xff;
	hw_interrupt(IF_SERIAL, IF_SERIAL);
	hw_interrupt(0, IF_SERIAL);
}
