#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>




// Loading ma_sound from memory:
// https://gist.github.com/p1xelHer0/abed4728096ea3a88af7802cbe46cf08


#include "miniaudio.h"
void ma_sleep(ma_uint32 milliseconds);

#define alloc_pointer(p) p = xalloc(sizeof *p)
#define GET_DECODER_CONFIG() const ma_decoder_config *decoder_config = &sound_config.decoder_config
#define GET_SOUND_ENGINE() ma_engine *engine = &sound_config.engine;

#define SOUND_OK(call) ((call) == MA_SUCCESS)
#define SOUND_FAILED(call) ((call) != MA_SUCCESS)
#define SOUND_CHANNEL_COUNT 32

#define SOUND_CHANNEL_UNINITIALIZED 0
#define SOUND_CHANNEL_PLAYING 1
#define SOUND_CHANNEL_PAUSED 2
#define SOUND_CHANNEL_STOPPED 3

#define CHECK(call) assert(call == MA_SUCCESS)


static MA_INLINE float SATURATE(float x)
{
    if (x < -1) return -1;
    if (x > +1) return +1;
    return x;
}

void *xalloc(size_t size)
{
    return calloc(size, 1);
}


typedef struct
{
    ma_decoder decoder;
    ma_sound sound_handle;
    int status;
    int current_loop;
    int max_loops;

    // A reference to its own index
    int index;

    // From ranges from 0 to 1
    float volume;

    // Pan ranges from -1 to 1
    float pan;

} Sound_Channel;

typedef struct
{
    ma_decoder decoder;
    ma_decoder_config config;

    
    // Private data, do not touch!
    void *data;
    size_t size;
} Sound;

typedef struct
{
    int rate;
    int channels;
    ma_format format;
    ma_decoder_config decoder_config;
    ma_device device;

    // Gloabal mutex
    ma_mutex mutex;

    ma_engine engine;

} Sound_Config;

Sound_Config sound_config;
Sound_Channel channels[SOUND_CHANNEL_COUNT] = {0};

void Sound_DeviceLock()
{
    ma_mutex_lock(&sound_config.mutex);
}

void Sound_DeviceUnlock()
{
    ma_mutex_unlock(&sound_config.mutex);
}

// Returns -1 on error
int Sound_FindFreeChannel()
{
    int result = -1;

    for(int i = 0; i < SOUND_CHANNEL_COUNT; i++)
    {
        Sound_Channel *c = &channels[i];
        if(c->status == SOUND_CHANNEL_STOPPED || c->status == SOUND_CHANNEL_UNINITIALIZED)
        {
            result = i;
            break;
        }
    }

    return result;
}

Sound_Channel *Sound_FindFreeChannelPointer()
{
    Sound_Channel *result = 0;
    
    int index = Sound_FindFreeChannel();
    if(index != -1)
    {
        result = &channels[index];
    }

    return result;
}

void data_callback(ma_device* pDevice, void* output, const void* input, ma_uint32 frame_count)
{
    ma_uint32 frames_remaining;
    float *output_f32;

    Sound_DeviceLock();
    


    // Todo: Make this global
    float output_buffer[32000] = {0};
    
    for(int i = 0; i < SOUND_CHANNEL_COUNT; i++)
    {
        frames_remaining = frame_count;
        output_f32 = output;
        Sound_Channel *c = &channels[i];
        if(c->status == SOUND_CHANNEL_PLAYING)
        {
            float volume = c->volume;
            float pan = c->pan;
            float pan_l = SATURATE(1 - pan);
            float pan_r = SATURATE(1 + pan);

            while(frames_remaining > 0)
            {
                int channel_count;
                int mono_samples;
                int i;

                ma_uint64 frames_read = 0;
                ma_decoder *decoder = &c->decoder;
                channel_count = decoder->outputChannels;
                ma_decoder_read_pcm_frames(decoder, output_f32, frames_remaining, &frames_read);
                frames_remaining -= (ma_int32)frames_read;
                mono_samples = frames_read * channel_count;

                // Do some mixing here.
                for(i = 0; i < mono_samples; i += 2)
                {
                    float l = output_f32[i + 0];
                    float r = output_f32[i + 1];

                    l *= volume;
                    r *= volume;

                    // Write them back
                    output_f32[i + 0] = l;
                    output_f32[i + 1] = r;
                }

                output_f32 += mono_samples;

                if(frames_read == 0)
                {
                    // We're at the end of the file
                    // If we're looping this sound, then try again
                    if(c->max_loops == -1 || c->current_loop < c->max_loops)
                    {
                        // Seek back to the start.
                        ma_decoder_seek_to_pcm_frame(decoder, 0);
                        printf("seek...\n");
                    }
                    else
                    {
                        c->status = SOUND_CHANNEL_STOPPED;
                        break;
                    }
                }
            }
        }
    }

    Sound_DeviceUnlock();
}

void data_callback2(ma_device* pDevice, void* output, const void* input, ma_uint32 frame_count)
{
    GET_SOUND_ENGINE();
    ma_engine_read_pcm_frames(engine, output, frame_count, NULL);
}

static void InitChannels()
{
    memset(channels, 0, sizeof(channels));

    for(int i = 0; i < SOUND_CHANNEL_COUNT; i++)
    {
        Sound_Channel *c = &channels[i];
        c->volume = 1;
    }
}

bool Sound_Init()
{
    int rate = 44100;
    int channels = 2;
    ma_format format = ma_format_f32;
    ma_device_config device_config;
    ma_engine_config engine_config;

    sound_config.rate = rate;
    sound_config.channels = channels;
    sound_config.format = format;
    ma_mutex_init(&sound_config.mutex);

    // Finally, init the config that all decoders from this point on will use.
    sound_config.decoder_config = ma_decoder_config_init(sound_config.format, sound_config.channels, sound_config.rate);

    device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format   = format;
    device_config.playback.channels = channels;
    device_config.sampleRate        = rate;
    device_config.dataCallback      = data_callback2;

    InitChannels();



    ma_device_init(NULL, &device_config, &sound_config.device);

    engine_config = ma_engine_config_init();
    engine_config.pDevice = &sound_config.device;
    engine_config.noAutoStart = MA_TRUE;

    CHECK(ma_engine_init(&engine_config, &sound_config.engine));
    CHECK(ma_engine_start(&sound_config.engine));
}

size_t get_file_size(FILE *fp)
{
    struct stat buf = {0};

    int fd = fileno(fp);
    fstat(fd, &buf);
    return buf.st_size;
}

Sound *Sound_Load(const char *filepath)
{
    FILE *fp = fopen(filepath, "rb");
    void *data = 0;
    size_t size = 0;
    Sound *sound;

    if(fp)
    {
        size = get_file_size(fp);
        data = malloc(size);
        fread(data, 1, size, fp);
    }
    else
    {
        return 0;
    }

    if(fp)
    {
        fclose(fp);
    }

    alloc_pointer(sound);
    GET_DECODER_CONFIG();
    sound->data = data;
    sound->size = size;

    return sound;
}

void Sound_Rewind(Sound *sound)
{
    Sound_DeviceLock();

    ma_decoder_seek_to_pcm_frame(&sound->decoder, 0);

    Sound_DeviceUnlock();
}

void cb_channel_end(void *user_data, ma_sound *handle)
{
    Sound_DeviceLock();

    Sound_Channel *c = (Sound_Channel *)user_data;

    Sound_DeviceUnlock();
}


int Sound_Play(Sound *sound, int channel)
{
    // Each channel has its own decoder.
    // Every time a channel is played, it gets a new decoder initialized.
    // Will that work, or do we need to copy them?
    Sound_Channel *c = 0;
    int channel_index = -1;

    Sound_DeviceLock();

    c = Sound_FindFreeChannelPointer(&channel_index);
    if(c)
    {
        // Todo:
        // Uninit the decoder if the channel doesn't have one.
        // Uninit the sound

        ma_uint32 flags;

        GET_DECODER_CONFIG();
        c->status = SOUND_CHANNEL_PLAYING;
        c->index = channel_index;
        ma_decoder_init_memory(sound->data, sound->size, decoder_config, &c->decoder);

        ma_data_source *data_source = c->decoder.ds.pCurrent;
        ma_sound_config config = ma_sound_config_init();
        config.pDataSource = data_source;
        config.endCallback = cb_channel_end;

        flags = MA_SOUND_FLAG_STREAM;
        GET_SOUND_ENGINE();
        CHECK(ma_sound_init_ex(engine, &config, &c->sound_handle));
        CHECK(ma_sound_start(&c->sound_handle));
    }

    Sound_DeviceUnlock();

    return channel_index;
}

int main()
{
    Sound_Init();
    Sound *sound = Sound_Load("test/blade.ogg");
    for(int i = 0; i < 3; i++)
    {
        Sound_Play(sound, -1);
        ma_sleep(300);
    }

    for(;;)
    {
        ma_sleep(1000);
    }
	
    return 0;
}
