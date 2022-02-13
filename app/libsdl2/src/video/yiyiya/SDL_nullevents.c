/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"
#include "screen.h"

#if SDL_VIDEO_DRIVER_DUMMY

/* Being a null driver, there's no event stream. We just define stubs for
   most of the API. */

#include "../../events/SDL_events_c.h"
#include "SDL_nullevents_c.h"
#include "SDL_nullvideo.h"

void DUMMY_PumpEvents(_THIS) {
   
  event_t e;
  int ret = event_poll(&e);
  if (ret <= 0) return;
  printf("poll event\n");

  //   e.mouse.y = screen->height - e.mouse.y;
  switch (e.type) {
    case MOUSE_MOTION:
      printf(".");

      break;
    case MOUSE_BUTTON_DOWN:
      printf("down\n");

      SDL_SendMouseButton(NULL, 0, SDL_PRESSED, SDL_BUTTON_MIDDLE);

      break;
    case MOUSE_BUTTON_UP:
      printf("up\n");
      break;

    case KEY_DOWN:
      SDL_SendKeyboardKey(SDL_PRESSED, e.key);

      break;
    case KEY_UP:
      SDL_SendKeyboardKey(SDL_RELEASED, e.key);
      break;
    default:
      break;
  }
  return;
}

#endif /* SDL_VIDEO_DRIVER_DUMMY */

/* vi: set ts=4 sw=4 expandtab: */
