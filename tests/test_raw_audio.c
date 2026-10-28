#include "sound_lib.h"

#include <unistd.h>
void sleep_ms(int ms)
{
    usleep(ms * 1e3);
}


int main()
{
    #define BASE_SFX_NAME test/blade
    #define BASE_MUS_NAME test/song
#define TEST_WAV

    #ifdef TEST_WAV
    #define EXT_NAME wav
    #endif

    #ifdef TEST_MP3
    #define EXT_NAME mp3
    #endif

    #ifdef TEST_OGG
    #define EXT_NAME ogg
    #endif

    #ifdef TEST_FLAC
    #define EXT_NAME flac
    #endif

    #ifndef EXT_NAME
    #error "Invalid test configuration"
    #endif

    const char *sfx_file = MA_XSTRINGIFY(BASE_SFX_NAME)"."MA_XSTRINGIFY(EXT_NAME);
    const char *mus_file = MA_XSTRINGIFY(BASE_MUS_NAME)"."MA_XSTRINGIFY(EXT_NAME);
    Sound_Buffer *sfx;
    Sound_Buffer *mus;
    int i;
    int sleep_millis = 2000;
    float playing_time_ms = 0;

    Audio_Init();

    sfx = SoundBuffer_Load("test/blade.wav");
    mus = SoundBuffer_Load(mus_file);

    if(sfx)
    {
        printf("Loaded '%s'\n", sfx_file);
    }
    else
    {
        printf("Could not load '%s'\n", sfx_file);
    }

    if(mus)
    {
        printf("Loaded '%s'\n", mus_file);
    }
    else
    {
        printf("Could not load '%s'\n", mus_file);
    }

    Channel_PlayEx(0, sfx, -1, 44100, ma_format_s16, 2, false);
    // Channel_SetPlayCursor(0, 5000);
    for(int i = 0; i < 100; i++)
    {
        float cursor = Channel_GetPlayCursor(0);
        printf("%f\n", cursor);
        sleep_ms(10);
    }
    playing_time_ms = Channel_GetPlayCursor(0);
    printf("Playing for %f\n", playing_time_ms);


	
    return 0;
}
