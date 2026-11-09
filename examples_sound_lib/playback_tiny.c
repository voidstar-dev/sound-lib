#include <stdio.h>
#include "sound_lib.h"
int main(int argc, char **argv)
{
    Sound_Buffer *buffer;
    Audio_Init();
    buffer = SoundBuffer_Load("resources/sample.wav");
    Channel_Play(0, buffer, 0);
    for(;;)
    {
        getchar();
    }
	
    return 0;
}
