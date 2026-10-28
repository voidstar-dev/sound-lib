/*******************************************************************************************
* Demonstrates initialisation and audio playback.
* A sound is loaded from disk and played back with the default audio device paraemeters.
* 2026-10-28
*******************************************************************************************/

#include <stdio.h>
#include "sound_lib.h"

int main()
{
    Sound_Buffer *buffer = 0;
    int loop_count = 0;

    /* Init the audio device with defaults */
    Audio_Init();

    /* Load a wave file to a memory buffer from disk */
    buffer = SoundBuffer_Load("resources/sample2.wav");

    /* Play that buffer on channel 0 without looping */
    Channel_Play(0, buffer, loop_count);

    for(;;)
    {
        getchar();
    }
	
    return 0;
}
