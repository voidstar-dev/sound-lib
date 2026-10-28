/*******************************************************************************************
* Demonstrates initialisation and audio playback with looping.
* A sound is loaded from disk and played back with the default audio device paraemeters.
* It continues looping until the program is terminated.
* Passing a negative number as the `loop_count` will loop the sound until the channel is stopped.
* 2026-10-28
*******************************************************************************************/

#include <stdio.h>
#include "sound_lib.h"

int main()
{
    Sound_Buffer *buffer = 0;

    /* Assigning a negative number to `loop_count` will loop the buffer */
    int loop_count = -1;

    /* Init the audio device with defaults */
    Audio_Init();

    /* Load a wave file to a memory buffer from disk.
       We make sure that it can be seamlessly looped.
    */
    buffer = SoundBuffer_Load("resources/microwave.wav");

    /* Play the buffer on channel 0, looping infinitely */
    Channel_Play(0, buffer, loop_count);

    for(;;)
    {
        getchar();
    }
	
    return 0;
}
