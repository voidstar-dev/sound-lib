#include "sound_api.h"

int main()
{
    #define BASE_SFX_NAME test/blade
    #define BASE_MUS_NAME test/song

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

    const char *sfx_file = MA_XSTRINGIFY(BASE_SFX_NAME)"."MA_XSTRINGIFY(EXT_NAME);
    const char *mus_file = MA_XSTRINGIFY(BASE_MUS_NAME)"."MA_XSTRINGIFY(EXT_NAME);
    Sound *sfx;
    Sound *mus;
    int i;
    int sleep_millis = 2000;

    Audio_Init();

    sfx = Sound_Load(sfx_file);
    mus = Sound_Load(mus_file);

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

    if(sfx)
    {
        printf("===================================================\n");
        printf("> Playing '%s\n", sfx_file);
        Channel_Start(-1, sfx, 0);
        printf("> Sleeping %d milliseconds\n", sleep_millis);
        printf("===================================================\n");
        ma_sleep(sleep_millis);
        printf("\n");
    }

    if(mus)
    {
        printf("===================================================\n");
        printf("> Playing '%s\n", mus_file);
        Channel_Start(-1, mus, 0);
        printf("> Sleeping %d milliseconds\n", sleep_millis);
        printf("===================================================\n");
        ma_sleep(sleep_millis);
    }

	
    return 0;
}
