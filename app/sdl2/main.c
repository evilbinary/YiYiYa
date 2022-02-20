#include <stdio.h>
#include <SDL.h>
#include <unistd.h>
#define true 1
#define false 0

#ifdef X86
#define WIDTH 1024
#define HEIGHT 768
#else

#define WIDTH 480
#define HEIGHT 272
#endif

int main( int argc, char* args[] )
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    // Initialize SDL.
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
            return 1;

    // Create the window where we will draw.
    window = SDL_CreateWindow("SDL_RenderClear",
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                    WIDTH, HEIGHT,
                    0);
    SDL_assert(window);
    // We must call SDL_CreateRenderer in order for draw calls to affect this window.
    renderer = SDL_CreateRenderer(window, -1, 0);

    // Select the color for drawing. It is set to red here.
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

    // Clear the entire screen to our selected color.
    SDL_RenderClear(renderer);

    // Up until now everything was drawn behind the scenes.
    // This will show the new, red contents of the window.
    SDL_RenderPresent(renderer);

    SDL_StartTextInput();
    // Give us time to see the window.
    //SDL_Delay(5000);
    int is_quit = 0;
    while (!is_quit) {
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            printf("sdl event %d\n", event.type);    
            switch (event.type) {
            case SDL_QUIT:
                is_quit = 1;
                break;
            case SDL_MOUSEMOTION:
                printf("SDL mouse motion: x=%d,y=%d\n", event.motion.x, event.motion.y);
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    printf("SDL button left down\n");
                    SDL_ShowCursor(false);
                } else if (event.button.button == SDL_BUTTON_MIDDLE) {
                    printf("SDL button middle down\n");
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    printf("SDL button right down\n");
                    SDL_ShowCursor(true);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    printf("SDL button left up\n");
                } else if (event.button.button == SDL_BUTTON_MIDDLE) {
                    printf("SDL button middle up\n");
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    printf("SDL button right up\n");
                }
                break;
            case SDL_MOUSEWHEEL:
                printf("SDL wheel dir:%d\n", event.wheel.direction);
                printf("SDL wheel x:%d, y:%d\n", event.wheel.x, event.wheel.y);
                break;
            case SDL_KEYDOWN:
                printf("SDL key down sym:%x(%s), mod:%x\n", event.key.keysym.sym, SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.mod);
                break;
            case SDL_KEYUP:
                printf("SDL key up sym:%x(%s), mod:%x\n", event.key.keysym.sym, SDL_GetKeyName(event.key.keysym.sym), event.key.keysym.mod);
                break;
            case SDL_WINDOWEVENT:
                printf("SDL window event %d\n", event.window.event);
                break;
            case SDL_TEXTINPUT:
                printf("SDL text input %s\n", event.text.text);
                break;
            default:
                break;
            }
        }
        SDL_Delay(10);
    }
    printf("sdl exit\n");
    SDL_Quit();
    return 0;
}
