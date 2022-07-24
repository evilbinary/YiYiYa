#include "io.h"
#include "../include/sys.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>


struct io_handler nix_io_hdlr;

int io_setup(const char* data)
{
	if(!strncmp(data, "pipe:", 5))
	{
		const char* path = data + 5;
		debugf("Setting up pipe-io\n");
		io_setup_pipe(path, &nix_io_hdlr);
	}
	else if(!strncmp(data, "network:", 8))
	{
		const char* path = data + 8;
		debugf("Setting up network-io\n");
		io_setup_network(path, &nix_io_hdlr);
	}
	else if (data[0] == 0)
	{
		debugf("Setting up none-io\n");
		io_setup_none(data, &nix_io_hdlr);
	}
	else
	{
		return 0;
	}
	io_handler_set(&nix_io_hdlr);
	return 1;
}
