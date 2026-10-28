/*******************************************************************************************
* Demonstrates initialisation and slightly more advanced audio playback.
* A sound is loaded from disk and played back with a user-specified set of audio parameters.
* When `raw_audio` is false, the buffer should be in some kind of audio container
* (wav, mp3, ogg, ...)
* in order for the audio decoder to know how to handle the data. 
* 2026-10-28
*******************************************************************************************/

#include <stdio.h>
#include "sound_lib.h"

int main()
{
    Sound_Buffer *buffer = 0;
    int loop_count = 0;
    bool raw_audio = false;
    int rc = 0;

    /* Request this format */
    ma_format src_format = ma_format_s16;
    
    /* Request mono playback */
    int src_channels = 1;

    /* Request this sample rate */
    int src_rate = 44100;

    /* Init the audio device with defaults */
    Audio_Init();

    /*Load a wave file to a memory buffer from disk. */
    buffer = SoundBuffer_Load("resources/sample2.wav");

    /* Play the buffer on channel 0 without looping */
    Channel_PlayEx(0, buffer, loop_count, src_rate, src_channels, src_format, raw_audio);

    for(;;)
    {
        getchar();
    }
	
    return 0;
}
