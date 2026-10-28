#include <stdio.h>
#include "sound_lib.h"

int main()
{
    Sound_Buffer *buffer = 0;
    int loop_count = 0;

    /* Init the audio device with defaults */
    Audio_Init();

    /* Load a wave file to a memory buffer from disk */
    buffer = SoundBuffer_Load("resources/song.wav");

    /* Play that buffer on channel 0 without looping */
    Channel_Play(0, buffer, loop_count);

    for(;;)
    {
        getchar();
    }
	
    return 0;
}
