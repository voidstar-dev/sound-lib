#include <SDL2/SDL.h>
#include "sound_lib.h"

// Uses SDL2
// We're not planning to ship SDL2 here, so this is not required to compile

void draw_centered_rect(SDL_Renderer *renderer, int x, int y, int radius, SDL_Color color)
{
    SDL_Rect rect;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    rect.x = x - radius;
    rect.y = y - radius;
    rect.w = radius * 2;
    rect.h = radius * 2;

    SDL_RenderFillRect(renderer, &rect);
}

int main()
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int mouse_x = 0;
    int mouse_y = 0;
    Sound_Buffer *buffer;
    int target_channel = 0;
    const char *filename = "test/song.wav";
    int channel_result;
    SDL_Color color1 = {0x00, 0xFF, 0x00, 0xFF};
    SDL_Color color2 = {0x00, 0x00, 0xFF, 0xFF};

    SDL_Init(SDL_INIT_VIDEO);
    Audio_Init();


    window = SDL_CreateWindow(
            "test_mouse",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800,
            600,
            SDL_WINDOW_HIDDEN);


    renderer = SDL_CreateRenderer(window, -1, 0);
    buffer = SoundBuffer_Load(filename);
    
    if(!buffer)
    {
        printf("Could not load file: %s\n", filename);
        exit(0);
    }

    SDL_ShowWindow(window);

    if(Channel_Start(target_channel, buffer, -1) != target_channel)
    {
        printf("Could not start target channel %d\n", target_channel);
        exit(0);
    }

    SDL_GetMouseState(&mouse_x, &mouse_y);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    for(;;)
    {
        SDL_Event event;
        int window_w, window_h;
        float px, py;
        int scancode;
        float pan, pitch;
        int rect_radius = 30;

        SDL_WaitEvent(0);

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    exit(0);
                break;

                case SDL_MOUSEMOTION:
                    mouse_x = event.motion.x;
                    mouse_y = event.motion.y;
                break;

                case SDL_KEYDOWN:
                    scancode = event.key.keysym.scancode;
                    if(scancode == SDL_SCANCODE_ESCAPE)
                    {
                        exit(0);
                    }
                break;
            }
        }

        SDL_GetWindowSize(window, &window_w, &window_h);
        px = (float)mouse_x / (float)window_w;
        py = 1 - ((float)mouse_y / (float)window_h); // Invert y

        // Remap to from [0, 1] to [-1, 1]
        pan = px * 2 - 1;

        // Remap from [0, 1] to [0.5, 1]
        pitch = py * 0.5 + 0.5;

        Channel_SetPan(target_channel, pan);
        Channel_SetPitch(target_channel, pitch);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
        SDL_RenderClear(renderer);

        // Draw our rectangles
        draw_centered_rect(renderer, window_w / 2, window_h / 2, rect_radius, color1);
        draw_centered_rect(renderer, mouse_x, mouse_y, rect_radius, color2);

        SDL_RenderPresent(renderer);
    }

	
    return 0;
}
