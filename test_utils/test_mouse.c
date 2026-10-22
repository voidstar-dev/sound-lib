/*
   A test application that demonstrates the following capabilities:
   - Sound loading / playback
   - Panning
   - Pitch
   - Volume
   - Sound looping
 
   Uses SDL2
   We're not planning to ship SDL2 here, so this is not required to compile

*/

#include <SDL2/SDL.h>
#include "sound_lib.h"

void compute_centered_rect(SDL_Rect *rect, int x, int y, int w, int h)
{
    rect->x = x - w / 2;
    rect->y = y - h / 2;
    rect->w = w;
    rect->h = h;
}


void draw_centered_rect(SDL_Renderer *renderer, int x, int y, int radius, SDL_Color color)
{
    SDL_Rect rect;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    compute_centered_rect(&rect, x, y, radius * 2, radius * 2);
    SDL_RenderFillRect(renderer, &rect);
}


void draw_centered_rect_with_border(SDL_Renderer *renderer, int x, int y, int radius, SDL_Color color, int border_size)
{
    SDL_Rect rect;
    int i;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    compute_centered_rect(&rect, x, y, radius * 2, radius * 2);

    for(i = 0; i < border_size; i++)
    {
        SDL_RenderDrawRect(renderer, &rect);
        rect.x += 1;
        rect.y += 1;
        rect.w -1;
        rect.h -1;
    }
}

float saturate(float f)
{
    if(f < 0) f = 0;
    if(f > 1) f = 1;

    return f;
}

float calculate_volume(int mouse_x, SDL_Rect rect)
{
    int left = rect.x;
    int delta = mouse_x - left;
    int w = rect.w;
    float volume;

    if(delta < 0) delta = 0;
    if(delta > w) delta = w;
    volume = (float)delta / (float)w;
    volume = saturate(volume);

    return volume;
}

void compute_volume_indicator_rect(SDL_Rect *rect, SDL_Rect volume_rect, SDL_Rect volume_indicator_rect, float volume)
{
    SDL_Rect tmp = volume_indicator_rect;
    int w = volume_indicator_rect.w;
    int w2 = volume_rect.w;
    int space = (w2 - w);
    int x = space * volume;
    tmp.x = volume_rect.x + x;

    *rect = tmp;
}

void draw_volume_control(SDL_Renderer *renderer, SDL_Rect volume_rect, SDL_Rect volume_indicator_rect, float volume, int volume_hovered, int volume_rect_pressed)
{
    int vol_shade;
    // Volume rect
    if(volume_hovered && !volume_rect_pressed)
    {
        vol_shade = 0x90;
    }
    else
    {
        vol_shade = volume_rect_pressed ? 0xCC : 0x60;
    }
    SDL_SetRenderDrawColor(renderer, vol_shade, vol_shade, vol_shade, 0xFF);
    SDL_RenderFillRect(renderer, &volume_rect);

    // Compute volume indicator rect position
    {
        SDL_Rect tmp;
        compute_volume_indicator_rect(&tmp, volume_rect, volume_indicator_rect, volume);
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(renderer, &tmp);
    }
}

int main(int argc, char **argv)
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
    int first_mouse = 1;
    int window_w, window_h;
    int volume_rect_pressed = 0;
    float volume = 1;
    int lmb_held = 0;
    int lmb_x = 0;
    SDL_Rect volume_rect;
    SDL_Rect volume_indicator_rect;
    SDL_Rect mouse_rect;

    SDL_Init(SDL_INIT_VIDEO);
    Audio_Init();

    if(argc > 1)
    {
        filename = argv[1];
    }


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
    SDL_GetMouseState(&mouse_x, &mouse_y);
    SDL_GetWindowSize(window, &window_w, &window_h);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    mouse_x = window_w / 2;
    mouse_y = window_h / 2;
    compute_centered_rect(&volume_rect, window_w / 2, window_h / 2 + 150, 300, 30);
    compute_centered_rect(&volume_indicator_rect, window_w / 2, window_h / 2 + 150, 30, 50);

    if(Channel_Start(target_channel, buffer, -1) != target_channel)
    {
        printf("Could not start target channel %d\n", target_channel);
        exit(0);
    }

    for(;;)
    {
        SDL_Event event;
        float px, py;
        int scancode;
        float pan, pitch;
        int rect_radius = 30;
        SDL_Color border_color;
        SDL_Color pan_color1 = {0};
        SDL_Color pan_color2 = {0};
        unsigned int pan_shade_left, pan_shade_right;
        unsigned int border_shade = 0x80;
        unsigned int border_pixel;
        int mid_x, mid_y;
        int mouse_dx = 0, mouse_dy = 0;
        int vol_shade = 0xFF;
        int volume_hovered = 0;
        int mouse_radius = rect_radius - 10;

        compute_centered_rect(&mouse_rect, mouse_x, mouse_y, mouse_radius * 2, mouse_radius * 2);

        // This blocks until we get an event. Very important for getting a responsive UI.
        SDL_WaitEvent(0);

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                    exit(0);
                break;

                case SDL_MOUSEMOTION:
                {
                    SDL_Rect tmp;
                    compute_volume_indicator_rect(&tmp, volume_rect, volume_indicator_rect, volume);
                    {
                        SDL_Rect *a = &mouse_rect;
                        SDL_Rect *b = &volume_rect;
                        SDL_Rect *c = &tmp;

                        mouse_dx += event.motion.xrel;
                        mouse_dy += event.motion.yrel;

                        if(SDL_HasIntersection(a, b) || SDL_HasIntersection(a, c))
                        {
                            volume_hovered = 1;
                        }
                    }
                }

                break;

                case SDL_KEYDOWN:
                    scancode = event.key.keysym.scancode;
                    if(scancode == SDL_SCANCODE_ESCAPE)
                    {
                        exit(0);
                    }
                break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                {
                    if(event.button.button == SDL_BUTTON_LEFT)
                    {
                        lmb_held = event.button.state;
                    }

                    if(lmb_held)
                    {
                        // Check with the entire volume control
                        SDL_Rect tmp;
                        compute_volume_indicator_rect(&tmp, volume_rect, volume_indicator_rect, volume);
                        {
                            SDL_Rect *a = &mouse_rect;
                            SDL_Rect *b = &volume_rect;
                            SDL_Rect *c = &tmp;
                            if(SDL_HasIntersection(a, b) || SDL_HasIntersection(a, c))
                            {
                                volume_rect_pressed = 1;
                            }
                        }
                    }
                    else
                    {
                        volume_rect_pressed = 0;
                    }
                }
                break;
            }
        }
        mouse_x += mouse_dx;
        mouse_y += mouse_dy;

        SDL_GetWindowSize(window, &window_w, &window_h);
        mid_x = window_w / 2;
        mid_y = window_h / 2;

        // Constrain mouse to the window size
        // Off by 1 error on the max value??
        if(mouse_x < 0) mouse_x = 0;
        if(mouse_x > window_w) mouse_x = window_w;

        if(mouse_y < 0) mouse_y = 0;
        if(mouse_y > window_h) mouse_y = window_h;

        // Normalize mouse coordinates
        px = (float)mouse_x / (float)window_w;
        py = 1 - ((float)mouse_y / (float)window_h); // Invert y

        // Calculate "pitch" color
        border_pixel = border_shade * py + 0x40;
        border_color.r = border_pixel;
        border_color.g = border_pixel;
        border_color.b = border_pixel;
        border_color.a = 0xFF;


        // Remap to from [0, 1] to [-1, 1]
        pan = px * 2 - 1;

        // Remap from [0, 1] to [0.5, 1.5]
        pitch = py + 0.5;

        // Pan rect colors
        pan_shade_left  =  saturate(1 - pan) *  0x80;
        pan_shade_right = saturate(1 + pan)  * 0x80;
        pan_color1.r = pan_shade_left;
        pan_color2.r = pan_shade_right;

        if(volume_rect_pressed)
        {
            volume = calculate_volume(mouse_x, volume_rect);
        }

        // Perform the actual mixing modifications
        Channel_SetPan(target_channel, pan);
        Channel_SetPitch(target_channel, pitch);
        Channel_SetVolume(target_channel, volume);

        // Draw
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
        SDL_RenderClear(renderer);

        draw_centered_rect_with_border(renderer, mid_x, mid_y, rect_radius * 2, border_color, 4);
        draw_centered_rect(renderer, mid_x, mid_y, rect_radius, color1);
        draw_centered_rect(renderer, 0 + (rect_radius * 2), mid_y, rect_radius, pan_color1);
        draw_centered_rect(renderer, window_w - (rect_radius * 2), mid_y, rect_radius, pan_color2);
        draw_volume_control(renderer, volume_rect, volume_indicator_rect, volume, volume_hovered, volume_rect_pressed);

        // The cursor rect
        draw_centered_rect(renderer, mouse_x, mouse_y, mouse_radius, color2);

        SDL_RenderPresent(renderer);
    }

    return 0;
}
