/*
 * sdl-audio.c
 * sdl 2 audio interface
 *
 * (C) 2001 Laguna
 * (C) 2020 Alex Oberhofer <alexmoberhofer@gmail.com>
 * 
 * Contributors:
 *  - Ryzee119 - SDL Fixes
 *
 * Licensed under the GPLv2, or later.
 */

#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "rc.h"
#include "pcm.h"
#include "sys.h"

struct pcm pcm;

static int samplerate = 44100;
static int stereo = 1;
static int sound = 0;
static int audio_started = 0;
static volatile int audio_done;

static SDL_AudioDeviceID device;

rcvar_t pcm_exports[] =
{
	RCV_BOOL("sound", &sound),
	RCV_INT("stereo", &stereo),
	RCV_INT("samplerate", &samplerate),
	RCV_END
};

//Callback for SDL AudioSpec 
static void audio_callback(void *blah, byte *stream, int len)
{
	memcpy(stream, pcm.buf, len);
	audio_done = 1;
}

void pcm_init()
{
	if(sound) 
	{
		SDL_AudioSpec want, obtained;

		if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) 
		{
			printf("WARNING: SDL Audio failed: %s\n", SDL_GetError());
			return;
		}

		want.freq = samplerate;
		want.format = AUDIO_U8;
		want.channels = 1 + stereo;
		want.samples = samplerate / 60;
		int i;
		for (i = 1; i < want.samples; i <<= 1);
		want.samples = i;
		want.callback = audio_callback;

		device = SDL_OpenAudioDevice(NULL, 0, &want, &obtained, 0);

		if(device == 0) 
		{
			printf("WARNING: SDL could not open audio device: %s\n", SDL_GetError());
			pcm_close();
			return;
		}

		pcm.hz = obtained.freq;
		pcm.stereo = obtained.channels - 1;
		pcm.len = obtained.size;
		pcm.buf = malloc(pcm.len);
		pcm.pos = 0;
		memset(pcm.buf, 0, pcm.len);
	}
	
}

int pcm_submit()
{
	if (!pcm.buf)
		return 0;
	if (pcm.pos < pcm.len)
		return 1;

	if(sound && !audio_started) 
	{ 
		SDL_PauseAudioDevice(device, 0);
		audio_started = 1;
	}

	while (!audio_done)
		sys_sleep(1000);

	audio_done = 0;
	pcm.pos = 0;

	return 1;
}

void pcm_close()
{
	if (sound)
		SDL_CloseAudioDevice(device);
}

void pcm_pause()
{
    if(sound)
        SDL_PauseAudioDevice(device, 1);
}

void pcm_resume()
{
    if(sound)
        SDL_PauseAudioDevice(device, 0);
}