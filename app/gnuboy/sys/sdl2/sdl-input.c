/*
 * sdl-input.c
 * sdl 2 joystick interfaces
 * 
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

#include "input.h"
#include "rc.h"
#include "defs.h"
#include "mem.h"
#include "sys.h"

static int joy_enable = 1;
static int joy_rumble_strength = 100; //0 to 100%
static int joy_deadzone = 40; //0 to 100%
static int joy_trace = 0;
static int alert = 0;
static int altenter = 0;



rcvar_t joy_exports[] =
    {
        RCV_BOOL("joy", &joy_enable),
        RCV_INT("joy_rumble_strength", &joy_rumble_strength),
        RCV_INT("joy_deadzone", &joy_deadzone),
        RCV_INT("alert_on_quit", &alert),
        RCV_INT("altenter", &altenter),
        RCV_INT("joy_trace", &joy_trace),
        RCV_END
    };

//Covert SDL KeyCodes to gnuboy button events
//This only needs to handle non standard ascii buttons.
static int kb_sdlkeycode_to_gnuboy(SDL_Keycode keycode)
{
        static const int lookup[49][2] =
        {
            //local event, sdl keycode
            {K_SHIFT, SDLK_LSHIFT},
            {K_SHIFT, SDLK_RSHIFT},
            {K_CTRL, SDLK_LCTRL},
            {K_CTRL, SDLK_RCTRL},
            {K_ALT, SDLK_LALT},
            {K_ALT, SDLK_RALT},
            {K_UP, SDLK_UP},
            {K_DOWN, SDLK_DOWN},
            {K_RIGHT, SDLK_RIGHT},
            {K_LEFT, SDLK_LEFT},
            {K_INS, SDLK_INSERT},
            {K_HOME, SDLK_HOME},
            {K_END, SDLK_END},
            {K_PRIOR, SDLK_PRIOR},
            {K_SYSRQ, SDLK_SYSREQ},
            {K_PAUSE, SDLK_PAUSE},
            {K_CAPS, SDLK_CAPSLOCK},
            {K_SCROLL, SDLK_SCROLLLOCK},
            {K_F1, SDLK_F1},
            {K_F2, SDLK_F2},
            {K_F3, SDLK_F3},
            {K_F4, SDLK_F4},
            {K_F5, SDLK_F5},
            {K_F6, SDLK_F6},
            {K_F7, SDLK_F7},
            {K_F8, SDLK_F8},
            {K_F9, SDLK_F9},
            {K_F10, SDLK_F10},
            {K_F11, SDLK_F11},
            {K_F12, SDLK_F12},
            {K_NUM0, SDLK_KP_0},
            {K_NUM1, SDLK_KP_1},
            {K_NUM2, SDLK_KP_2},
            {K_NUM3, SDLK_KP_3},
            {K_NUM4, SDLK_KP_4},
            {K_NUM5, SDLK_KP_5},
            {K_NUM6, SDLK_KP_6},
            {K_NUM7, SDLK_KP_7},
            {K_NUM8, SDLK_KP_8},
            {K_NUM9, SDLK_KP_9},
            {K_NUMPLUS, SDLK_KP_PLUS},
            {K_NUMMINUS, SDLK_KP_MINUS},
            {K_NUMMUL, SDLK_KP_MULTIPLY},
            {K_NUMDIV, SDLK_KP_DIVIDE},
            {K_NUMDOT, SDLK_KP_PERIOD},
            {K_NUMENTER, SDLK_KP_ENTER},
            {K_NUMLOCK, SDLK_NUMLOCKCLEAR},
            {K_TAB, SDLK_TAB},
            {'/', SDLK_KP_DIVIDE}
        };

    //Already a standard ascii character. Just leave
    if (keycode < MAX_KEYS)
        return keycode;

    //Find the matching gnuboy event from the sdl keycode then return it
    for (int i = 0; i < sizeof(lookup) / sizeof(lookup[0]); i++)
    {
        if (lookup[i][1] == keycode)
            return lookup[i][0];
    }
    return keycode;
}

//The function of K_JOYx can be modified via a rc file. This allows
//remapping of the gamecontroller inputs. See main.c for default mapping.
static Sint32 gamecontroller_map[14][2] =
    {
        {K_JOY0, SDL_CONTROLLER_BUTTON_A},
        {K_JOY1, SDL_CONTROLLER_BUTTON_B},
        {K_JOY2, SDL_CONTROLLER_BUTTON_BACK},
        {K_JOY3, SDL_CONTROLLER_BUTTON_START},
        {K_JOYUP, SDL_CONTROLLER_BUTTON_DPAD_UP},
        {K_JOYDOWN, SDL_CONTROLLER_BUTTON_DPAD_DOWN},
        {K_JOYLEFT, SDL_CONTROLLER_BUTTON_DPAD_LEFT},
        {K_JOYRIGHT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT},
        {K_JOY4, SDL_CONTROLLER_BUTTON_X},
        {K_JOY5, SDL_CONTROLLER_BUTTON_Y},
        {K_JOY6, SDL_CONTROLLER_BUTTON_LEFTSHOULDER},
        {K_JOY7, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER},
        {K_JOY8, SDL_CONTROLLER_BUTTON_LEFTSTICK},
        {K_JOY9, SDL_CONTROLLER_BUTTON_RIGHTSTICK}
    };

static int joy_find_gamecontroller_mapping(SDL_GameControllerButton button)
{
    for (int i = 0; i < (sizeof(gamecontroller_map) / sizeof(gamecontroller_map[0])); i++)
    {
        if (button == gamecontroller_map[i][1])
            return gamecontroller_map[i][0];
    }
    return -1;
}

void joy_init()
{
    //we obviously have no business being in here
    if (!joy_enable) return;

    //init gamecontroller (and joystick) subsystem
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
    {
        printf("JOY:    SDL could not initialize Joystick! SDL Error: %s\n", SDL_GetError());
    }

    if (joy_rumble_strength > 100)
        joy_rumble_strength = 100;
        
    if (joy_deadzone > 100)
        joy_deadzone = 100;

    if (joy_trace)
    {
        printf("JOY:    Rumble strength set to: %i%%\n", joy_rumble_strength);
        printf("JOY:    Deadzone set to: %i%%\n", joy_deadzone);
        printf("JOY:    Joystick subsystem initialized succesfully\n");
    }

}

void joy_close()
{
    SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

void ev_poll()
{
    event_t ev;
    SDL_Event event;
    static SDL_GameController *pad = NULL;

    while(SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            if(alert > 0) 
            {
                if(!confirm_exit()) 
                    exit(1);
            } 
            else 
            {
                exit(1);
            }
        }

        /* Keyboard */
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
        {
            SDL_Scancode scancode = event.key.keysym.scancode;
            int keycode = kb_sdlkeycode_to_gnuboy(SDL_GetKeyFromScancode(scancode));


            if(joy_trace) printf("KEY:    Pressed: %s\n",
                   SDL_GetKeyName(SDL_GetKeyFromScancode(event.key.keysym.scancode)));

            //Handle alt enter
            if(altenter && event.type == SDL_KEYDOWN)
                if((event.key.keysym.sym == SDLK_RETURN) && (event.key.keysym.mod & KMOD_ALT))
                    vid_fullscreen_toggle();

            if (scancode == SDL_SCANCODE_ESCAPE && event.type == SDL_KEYDOWN)
            {
                if(alert > 0) 
                { 
                    if(!confirm_exit())
                        exit(1);
                } 
                else 
                {
                    exit(1);
                }
            }

            //If the keycode is > MAX_KEYS, its not a standard ascii button and its not in the kb_sdlkeycode_to_gnuboy map.
            if (keycode < MAX_KEYS)
            {
                ev.type = (event.type == SDL_KEYDOWN) ? EV_PRESS : EV_RELEASE;
                ev.code = keycode;
                ev_postevent(&ev);
            }
            else
            {
                printf("JOY:    Pressed unsupported button %s\n", SDL_GetScancodeName(scancode));
            }
        }
        /* Handle gamecontroller hotplugging */
        else if (event.type == SDL_CONTROLLERDEVICEADDED && joy_enable)
        {
            if(joy_trace) printf("JOY:    Controller connected with index: %d\n", event.cdevice.which);
            SDL_GameControllerOpen(event.cdevice.which);
            if(joy_trace) printf("JOY:    Opened controller: %s\n",
                                 SDL_GameControllerNameForIndex(event.cdevice.which));
        }
        else if (event.type == SDL_CONTROLLERDEVICEREMOVED && joy_enable)
        {
            if (joy_trace) printf("Controller %d disconnected\n", event.cdevice.which);
            if (pad == SDL_GameControllerFromInstanceID(event.cdevice.which))
                pad = NULL;
            SDL_GameControllerClose(SDL_GameControllerFromInstanceID(event.cdevice.which));
        }
        /* Handle gamecontroller button events*/
        else if ((event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP) && joy_enable)
        {
            //Update the pad pointer. This ensures that only the controller actively being used rumbles.
            pad = SDL_GameControllerFromInstanceID(event.cdevice.which);
            SDL_GameControllerButton btn = event.cbutton.button;
            const char *buttonstring = SDL_GameControllerGetStringForButton(btn);

            ev.type = (event.type == SDL_CONTROLLERBUTTONDOWN) ? EV_PRESS : EV_RELEASE;
            ev.code = joy_find_gamecontroller_mapping(btn);

            if (joy_trace)
                printf("JOY:    %s: %s\n", (event.type == SDL_CONTROLLERBUTTONDOWN)
                    ? "Pressed" : "Released", buttonstring);

            if (ev.code != -1)
                ev_postevent(&ev);
        }
        /* Handle gamecontroller axis events. Convert axis to dpad events */
        else if (event.type == SDL_CONTROLLERAXISMOTION && joy_enable)
        {
            //Track the previous hat state so that we only trigger input events on hat changes.
            static Uint8 old_hat = 0;
            //Prevent button mashing at the deadzone area.
            int hysteresis = 2000;
            int deadzone = joy_deadzone * (0x7FFF / 100);
            int press_ev = 0, release_ev = 0;
            SDL_ControllerAxisEvent ae = event.caxis;

            //Create a dpad press event if the axis is greater than the deadzone and that direction has changed since last update.
            press_ev = (ae.axis == SDL_CONTROLLER_AXIS_LEFTX && ae.value < -deadzone && !(old_hat & SDL_HAT_LEFT))  ? SDL_CONTROLLER_BUTTON_DPAD_LEFT  :
                       (ae.axis == SDL_CONTROLLER_AXIS_LEFTX && ae.value >  deadzone && !(old_hat & SDL_HAT_RIGHT)) ? SDL_CONTROLLER_BUTTON_DPAD_RIGHT :
                       (ae.axis == SDL_CONTROLLER_AXIS_LEFTY && ae.value < -deadzone && !(old_hat & SDL_HAT_UP))    ? SDL_CONTROLLER_BUTTON_DPAD_UP    :
                       (ae.axis == SDL_CONTROLLER_AXIS_LEFTY && ae.value >  deadzone && !(old_hat & SDL_HAT_DOWN))  ? SDL_CONTROLLER_BUTTON_DPAD_DOWN  : 0;

            //Create a dpad release event if the axis is lss than the deadzone (with some hysteresis) and that direction has changed since last update.
            if (ae.value >= -(deadzone - hysteresis) && ae.value <= (deadzone - hysteresis))
            {
                release_ev = (ae.axis == SDL_CONTROLLER_AXIS_LEFTX && (old_hat & SDL_HAT_LEFT))  ? SDL_CONTROLLER_BUTTON_DPAD_LEFT  :
                             (ae.axis == SDL_CONTROLLER_AXIS_LEFTX && (old_hat & SDL_HAT_RIGHT)) ? SDL_CONTROLLER_BUTTON_DPAD_RIGHT :
                             (ae.axis == SDL_CONTROLLER_AXIS_LEFTY && (old_hat & SDL_HAT_UP))    ? SDL_CONTROLLER_BUTTON_DPAD_UP    :
                             (ae.axis == SDL_CONTROLLER_AXIS_LEFTY && (old_hat & SDL_HAT_DOWN))  ? SDL_CONTROLLER_BUTTON_DPAD_DOWN  : 0;
            }

            //We have a press or release event. Register it in the emulator.
            if (press_ev || release_ev)
            {
                //Is it a press or release event.
                ev.type = press_ev ? EV_PRESS : EV_RELEASE;
                //Assign it a gnuboy input code based on the SDL_GAMECONTROLLER_BUTTON.
                ev.code = joy_find_gamecontroller_mapping(press_ev ? press_ev : release_ev);
                //Clear or set the old_hat bitmask to update the current state.
                int new_hat = (ev.code == K_JOYLEFT)  ? SDL_HAT_LEFT  :
                              (ev.code == K_JOYRIGHT) ? SDL_HAT_RIGHT :
                              (ev.code == K_JOYUP)    ? SDL_HAT_UP    :
                              (ev.code == K_JOYDOWN)  ? SDL_HAT_DOWN  : 0;
                (press_ev) ? (old_hat |= new_hat) : (old_hat &= ~new_hat);

                if (joy_trace)
                    printf("JOY:    Analog stick hat - %s: %s\n", (press_ev) ? "Pressed" : "Released",
                                                        SDL_GameControllerGetStringForButton(press_ev ? press_ev : release_ev));

                ev_postevent(&ev);
            }
        }
    }

    //Will rumble if playing a compatible rom. The rumble strength can be configured by changing the rumble_strength rc var.
    static int old_rumble_state = 0;
    if (pad != NULL && mbc.type == MBC_RUMBLE && (joy_rumble_strength > 0) && (mbc.rumble_state != old_rumble_state))
    {
        Uint16 rumble_val = (mbc.rumble_state) ? ((0xFFFF / 100) * joy_rumble_strength) : 0x0000;
        if (SDL_GameControllerRumble(pad, rumble_val, rumble_val, 250) == 0)
        {
            old_rumble_state = mbc.rumble_state;
        }
    }

}

//Confirm exit upon window close or escape press
int confirm_exit()
{
    pcm_pause();
    int buttonid;

    const SDL_MessageBoxButtonData buttons[] = {
            { 0, 0, "Quit" },
            { 0, 1, "Resume" },
    };

    const SDL_MessageBoxData messageboxdata = {
            SDL_MESSAGEBOX_INFORMATION,
            NULL, 
            "SDL2-GNUBoy", 
            "Are you sure you want to quit?",
            SDL_arraysize(buttons),
            buttons,
            NULL 
    };

    if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0)
        if(joy_trace) printf("MSGBOX: Error displaying message box\n");

    if (buttonid != -1)
    {
        if(joy_trace)
            printf("MSGBOX: Selection was %s, with id %d\n", buttons[buttonid].text, buttons[buttonid].buttonid);

        pcm_resume();
        return buttons[buttonid].buttonid;
    }

    pcm_resume();
    return -1;
}
