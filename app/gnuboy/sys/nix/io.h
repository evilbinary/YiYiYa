#ifndef NIX_IO_H
#define NIX_IO_H

#include <io.h>

int io_setup_pipe(const char* link, struct io_handler* io_hdlr);
int io_setup_network(const char* link, struct io_handler* io_hdlr);
int io_setup_none(const char* link, struct io_handler* io_hdlr);

#endif
