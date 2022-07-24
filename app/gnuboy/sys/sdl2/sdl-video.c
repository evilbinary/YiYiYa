/*
 * sdl-video.c
 * sdl 2 interfaces -- based on sdl.c 
 *
 * (C) 2001 Damian Gryski <dgryski@uwaterloo.ca>
 * (C) 2020 Alex Oberhofer <alexmoberhofer@gmail.com>
 * 
 * Contributors:
 *  - Ryzee119 - SDL Fixes / Integer Scaling
 *
 * Licensed under the GPLv2, or later.
 */

#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "fb.h"
#include "rc.h"
#include "sys.h"

struct fb fb;

static SDL_Window *window;
static SDL_Texture *texture;
static SDL_Renderer *renderer;

static int fullscreen = 0;
static int scale = 1;
static int integer_scale = 1;
static int render_type = 1; // 0: Software || 1: Default HW
static int render_trace = 0;

static int vmode[3] = {0, 0, 32};
static byte pix[160 * 144 * sizeof(uint32_t)];

rcvar_t vid_exports[] =
{
	RCV_VECTOR("vmode", &vmode, 3),
	RCV_BOOL("fullscreen", &fullscreen),
    RCV_BOOL("vid_trace", &render_trace),
    RCV_INT("render_type", &render_type),
	RCV_END
};

void vid_init()
{
	if (!vmode[0] || !vmode[1])
	{
		scale = rc_getint("scale");
		
		if (scale < 1) scale = 1;

		vmode[0] = 160;
		vmode[1] = 144;
	}

	if (SDL_Init(SDL_INIT_VIDEO ) < 0) //| SDL_INIT_TIMER
	{
		printf("VID:    SDL could not initialize Video! SDL Error: %s\n", SDL_GetError());
		exit(1);
	}
	else
	{
		window = SDL_CreateWindow("SDL2 GNUBoy", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   vmode[0] * scale, vmode[1] * scale,
                                  (fullscreen > 0) ? SDL_WINDOW_FULLSCREEN_DESKTOP: SDL_WINDOW_RESIZABLE);

        if(render_type) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        else renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

		if (!renderer) //Software fallback
        {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
            render_type = 0;
        }
			
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_BGRA32,
                                    SDL_TEXTUREACCESS_STREAMING, vmode[0], vmode[1]);

		SDL_RenderSetLogicalSize(renderer, vmode[0], vmode[1]);
		integer_scale = rc_getint("integer_scale");
		if (integer_scale)
		{
			int window_width, window_height, render_width, render_height;
			SDL_GetRendererOutputSize(renderer, &window_width, &window_height);
			SDL_RenderGetLogicalSize(renderer, &render_width, &render_height);
			SDL_bool makes_sense = (window_width >= render_width && window_height >= render_height);
			SDL_RenderSetIntegerScale(renderer, makes_sense);
		}
	}

	fb.w = vmode[0];
	fb.h = vmode[1];
	fb.pelsize = 4;
	fb.pitch = fb.w * fb.pelsize;
	fb.indexed = fb.pelsize == 1;
	fb.ptr = pix;
	fb.cc[0].r = fb.cc[1].r = fb.cc[2].r = fb.cc[3].r = 0;
	fb.cc[0].l = 16;
	fb.cc[1].l = 8;
	fb.cc[2].l = 0;
	fb.cc[3].l = 0;

	fb.enabled = 1;
	fb.dirty = 0;

	if(render_trace)
    {
        SDL_RendererInfo r_info;
        SDL_GetRendererInfo(renderer, &r_info);

        printf("VID:    Using render mode: %s\n",
               (render_type) ? "Hardware Accelerated Renderer" : "Software Renderer");
        printf("VID:    Using render driver: %s\n", r_info.name);
        printf("VID:    Using internal resolution: %d x %d\n", vmode[0], vmode[1]);
        printf("VID:    Fullscreen set to: %d\n", fullscreen);
        printf("VID:    Scale set to: %d\n", scale);
        printf("VID:    Integer scale set to: %d\n", integer_scale);
    }

	joy_init();
}

void vid_setpal(int i, int r, int g, int b)
{
}

void vid_preinit()
{
}

void vid_close()
{
	joy_close();
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(texture);
	SDL_Quit();
}

void vid_settitle(char *title)
{
	SDL_SetWindowTitle(window, title);
}

void vid_begin()
{
	fb.ptr = pix;
}

void vid_fullscreen_toggle()
{
	if(!fullscreen)
	{
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		fullscreen = 1;
	} 
	else 
	{
		SDL_SetWindowFullscreen(window, 0);
		fullscreen = 0;
	}
	
}

void vid_end()
{
	fb.ptr = pix;
	if(fb.enabled)
	{
		SDL_RenderClear(renderer);
		SDL_UpdateTexture(texture, NULL, pix, vmode[0] * sizeof(uint32_t));
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

}



void vid_screenshot()
{
    //FIXME: Only works under software rendering
    //FIXME: Doesnt work correctly over fullscreen in sw rendering
    if(!render_type && !fullscreen)
    {
        SDL_Surface *surface = SDL_GetWindowSurface(window);
        if(surface)
        {
            SDL_RenderReadPixels(renderer, NULL, SDL_PIXELFORMAT_BGRA32,
                                 surface->pixels, surface->pitch);
            SDL_SaveBMP(surface, "screenshot.bmp");
        }
        else printf("VID:    SDL Error: %s\n", SDL_GetError());

        SDL_FreeSurface(surface);
    }
    else printf("VID:   Screenshot feature unsupported using hardware renderer or fullscreen mode.\n");

}