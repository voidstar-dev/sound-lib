/*******************************************************************************************
* Demonstrates initialisation and playback from raw, uncompressed audio data.
* A sound is loaded from disk and played back with a user-specified set of audio parameters.
* When playing raw audio, the rate, channels, and format _should_ match the source data
* for correct playback.
* Mismatches _are_ allowed, but the playback may not be reproduced correctly.
* Of these parameters, the most important to get right is the format.
* 2026-10-28
*******************************************************************************************/

#include <stdio.h>
#include "sound_lib.h"

int main()
{
    Sound_Buffer *buffer = 0;
    int loop_count = 0;

    /* We know the parameters in advance. This is just sample2.wav without the WAVE header. */
    ma_format src_format = ma_format_s16;
    int src_channels = 2;
    int src_rate = 44100;
    bool raw_audio = true;

    /* Init the audio device with defaults */
    Audio_Init();

    /* Load an uncompressed, raw audio file to a memory buffer from disk. */
    buffer = SoundBuffer_Load("resources/sample2.raw");

    /* Play the buffer on channel 0 without looping */
    Channel_PlayEx(0, buffer, loop_count, src_rate, src_channels, src_format, raw_audio);

    for(;;)
    {
        getchar();
    }
	
    return 0;
}
