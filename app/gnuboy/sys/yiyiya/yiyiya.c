/*
 * nix.c
 *
 * System interface for *nix systems.
 */
#undef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include <SDL2/SDL.h>

#include "defs.h"
#include "rc.h"

void *sys_timer()
{
	Uint32 *tv;
	
	tv = malloc(sizeof *tv);
	*tv = SDL_GetTicks() * 1000;
	return tv;
}

int sys_elapsed(Uint32 *cl)
{
	Uint32 now;
	Uint32 usecs;

	now = SDL_GetTicks() * 1000;
	usecs = now - *cl;
	*cl = now;
	return usecs;
}

void sys_sleep(int us)
{
	if(us <= 0) return;
	
	SDL_Delay(us/1000);
}

void sys_checkdir(char *path, int wr)
{
}

void sys_initpath()
{
    char *buf = ".";

    if (rc_getstr("rcpath") == NULL)
        rc_setvar("rcpath", 1, &buf);

    if (rc_getstr("savedir") == NULL)
        rc_setvar("savedir", 1, &buf);
}

void sys_sanitize(char *s)
{
}




