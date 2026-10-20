#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


#include "miniaudio.h"

#define alloc_pointer(p) p = xalloc(sizeof *p)
#define GET_DECODER_CONFIG() const ma_decoder_config *decoder_config = &sound_config.decoder_config

#define SOUND_OK(call) ((call) == MA_SUCCESS)
#define SOUND_FAILED(call) ((call) != MA_SUCCESS)

void *xalloc(size_t size)
{
    return calloc(size, 1);
}


typedef struct
{
    ma_decoder *decoder;
    int playing;
} Sound_Channel;

typedef struct
{
    ma_decoder decoder;
    ma_decoder_config config;
} Sound;

typedef struct
{
    int rate;
    int channels;
    ma_format format;
    ma_decoder_config decoder_config;
    ma_device device;

} Sound_Config;

Sound_Config sound_config;
Sound_Channel channel;

void data_callback(ma_device* pDevice, void* output, const void* input, ma_uint32 frame_count)
{
    if(channel.playing)
    {
        ma_decoder *decoder = channel.decoder;
        ma_decoder_read_pcm_frames(decoder, output, frame_count, NULL);
        printf("Read...\n");
    }

}

bool Sound_Init()
{
    int rate = 44100;
    int channels = 2;
    ma_format format = ma_format_f32;
    ma_device_config device_config;

    sound_config.rate = rate;
    sound_config.channels = channels;
    sound_config.format = format;

    // Finally, init the config that all decoders from this point on will use.
    sound_config.decoder_config = ma_decoder_config_init(sound_config.format, sound_config.channels, sound_config.rate);

    device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format   = format;
    device_config.playback.channels = channels;
    device_config.sampleRate        = rate;
    device_config.dataCallback      = data_callback;

    ma_device_init(NULL, &device_config, &sound_config.device);
    if(SOUND_OK(ma_device_start(&sound_config.device)))
    {
    }
    else
    {
        printf("Failed...\n");
        exit(0);
    }
}

Sound *Sound_Load(const char *filepath)
{
    Sound *sound;
    alloc_pointer(sound);
    GET_DECODER_CONFIG();
    ma_decoder_init_file(filepath, decoder_config, &sound->decoder);

    return sound;
}

void Sound_Play(Sound *sound)
{
    channel.playing = true;
    channel.decoder = &sound->decoder;
}

int main()
{
    Sound_Init();
    Sound *sound = Sound_Load("test/blade.wav");
    Sound_Play(sound);

    for(;;)
    {
        sleep(1);
    }
	
    return 0;
}
