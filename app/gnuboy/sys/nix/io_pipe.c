#include <stdio.h>

#include "defs.h"
#include "io.h"
#include "hw.h"
#include "regs.h"
#include "debug.h"
#include "sys.h"

#include <unistd.h>
#include <stdio.h>
#include <poll.h>

#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include <string.h>
#include <stdlib.h>

#define FIFO_PERMISSIONS 0777

void io_pipe_init();
void io_pipe_init_path();
void io_pipe_try_open();
void io_pipe_send(un8);
void io_pipe_send_mode(int mode, un8* byte);
int  io_pipe_ready();
void io_pipe_recv();
int  io_pipe_read(un8* byte);
un8  io_pipe_trigger_int(un8 byte);
void io_pipe_shutdown();

int file_out, file_in, player_a;
const char* file_name_prefix;
char* file_name_out;
char* file_name_in;

int io_setup_pipe(const char* path, struct io_handler* io_hdlr)
{
	io_hdlr->init = io_pipe_init;
	io_hdlr->send = io_pipe_send;
	io_hdlr->recv = io_pipe_recv;
	io_hdlr->shutdown = io_pipe_shutdown;
	file_name_prefix = path;
	return 1;
}

void io_pipe_try_open()
{
	if (file_out == -1 && access(file_name_out, 0) != -1)
	{
		debugf("Opening file_out: %s\n", file_name_out);
		file_out = open(file_name_out, O_WRONLY | O_NONBLOCK);
	}
}

int dir_name(char* file_name)
{
	unsigned int i, last = -1;

	for (i = 0; i < strlen(file_name); i++)
	{
		if (file_name[i] == '/')
		{
			last = i;
		}
	}
	if (last == -1)
	{
		return 0;
	}
	file_name[last] = '\0';
	return 1;
}

void un_dir_name(char* file_name)
{
	file_name[strlen(file_name)] = '/';
}

void io_pipe_init_path()
{
	int max_file_name_length = strlen(file_name_prefix) + 1 + 2;

	char* dirname;

	struct stat dir;

	file_name_out = (char*) malloc(sizeof(char) * max_file_name_length);
	file_name_in = (char*) malloc(sizeof(char) * max_file_name_length);

	strncpy(file_name_out, file_name_prefix, max_file_name_length);
	strncpy(file_name_in, file_name_prefix, max_file_name_length);

	file_name_out[max_file_name_length - 3] = '_';
	file_name_out[max_file_name_length - 2] = 'a';
	file_name_out[max_file_name_length - 1] = '\0';

	file_name_in[max_file_name_length - 3] = '_';
	file_name_in[max_file_name_length - 2] = 'b';
	file_name_in[max_file_name_length - 1] = '\0';

	if (dir_name(file_name_out))
	{
		dirname = (char*) malloc(sizeof(char) * (strlen(file_name_out) + 1));
		strncpy(dirname, file_name_out, strlen(file_name_out)+1);
		un_dir_name(file_name_out);
	}
	else
	{
		dirname = getcwd(NULL, 0);
	}

	debugf("file_a dir name: '%s' length: %ld\n", dirname, strlen(dirname));

	if (!(stat(dirname, &dir) == 0 && S_ISDIR(dir.st_mode)))
	{
		fprintf(stderr, "link: failed to create pipe, directory does not exist\n");
		exit(1);
	}

	if (access(dirname, W_OK))
	{
		fprintf(stderr, "link: failed to create pipe, directory not writable\n");
		exit(1);
	}


	if (access(file_name_in, 0) != -1)
	{
		char* flip = file_name_in;
		file_name_in = file_name_out;
		file_name_out = flip;
	}

	if (access(file_name_in, 0) != -1)
	{
		fprintf(stderr, "link: failed to create pipe, file exists\n");
		exit(1);
	}

	if (dirname != NULL)
	{
		free(dirname);
	}
	debugf("in:  %s\n", file_name_in);
	debugf("out: %s\n",  file_name_out);

}

void io_pipe_init()
{
	file_in = -1;
	file_out = -1;

	io_pipe_init_path();

	mkfifo(file_name_in, FIFO_PERMISSIONS);

	file_in = open(file_name_in, O_RDONLY|O_NONBLOCK);
}

void io_pipe_send(un8 byte)
{
	io_pipe_send_mode(SEND_ACTIVE, &byte);
}

void io_pipe_send_mode(int mode, un8* byte)
{
	int polled, can_write, poll_err, poll_hup;
	struct pollfd poll_s;

	io_pipe_try_open();

	if (file_out != -1)
	{

		poll_s.fd = file_out;
		poll_s.events = POLLOUT;
		poll_s.revents = 0;

		polled = poll(&poll_s, 1, 0);
		can_write = poll_s.revents & POLLOUT;
		poll_err = poll_s.revents & POLLERR;
		poll_hup = poll_s.revents & POLLHUP;

		if (poll_err || poll_hup)
		{
			io_reset();
		}
		else if (polled && can_write)
		{
			write(file_out, byte, 1);

			if (mode)
			{
				debugf("Sending to %s %d %c\n", file_name_out, *byte, *byte);

				while (!io_pipe_read(byte)) { sys_sleep(1); } // TODO block until the other GB answers, in a smarter way

				debugf("Reading %d %c\n", *byte, *byte);

				io_pipe_trigger_int(*byte);
			}
			else
			{
				sys_sleep(25); // TODO if for some reason the second gameboy answers to fast "Mario Tennis" will think the link is interrupted, WHY? I  don't know, might have to do with the fact that this implementation transfers data in one cycle unlike the original GB and GBC
				debugf("Answering %s %d %c\n", file_name_out, *byte, *byte);
			}
		}
	}
}

int io_pipe_ready ()
{
	int polled, can_read, poll_err, poll_hup;
	struct pollfd poll_s;

	io_pipe_try_open();

	poll_s.fd = file_in;
	poll_s.events = POLLIN;
	poll_s.revents = 0;

	polled = poll(&poll_s, 1, 0);
	can_read = poll_s.revents & POLLIN;
	poll_err = poll_s.revents & POLLERR;
	poll_hup = poll_s.revents & POLLHUP;

	if (poll_err || poll_hup)
	{
		io_reset();
	}

	return polled == 1 && can_read != 0;
}

void io_pipe_recv ()
{
	if (io_pipe_ready())
	{
		un8 byte;
		if (io_pipe_read(&byte))
		{
			debugf("Reading %d %c\n", byte, byte);
			un8 own_data = io_pipe_trigger_int(byte);
			io_pipe_send_mode(SEND_PASSIVE, &own_data);
		}
	}
}

un8 io_pipe_trigger_int(un8 byte)
{
	un8 old_data = R_SB;
	R_SB = byte;
	R_SC = ((~128) & R_SC);
	hw_interrupt(IF_SERIAL, IF_SERIAL);
	hw_interrupt(0, IF_SERIAL);
	return old_data;
}

int io_pipe_read(un8* byte)
{
	int ret;
	ret = read(file_in, byte, 1);
	if (ret != 1 && errno != EAGAIN) {
		io_reset();
		fprintf(stderr, "Alarm\n");
	}
	return ret == 1;
}

void io_pipe_shutdown()
{
	debugf("io_pipe_shutdown\n");

	if ( file_in != -1 )
	{
		close(file_in);
	}

	if ( file_out != -1 )
	{
		close(file_out);
	}

	remove(file_name_in);

	free(file_name_in);
	free(file_name_out);
}
