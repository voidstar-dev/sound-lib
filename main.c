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

#define SOUND_CHANNEL_STATUS_UNINITIALIZED 0
#define SOUND_CHANNEL_STATUS_PLAYING 1
#define SOUND_CHANNEL_STATUS_PAUSED 2
#define SOUND_CHANNEL_STATUS_STOPPED 3

#define CHECK(call) assert(call == MA_SUCCESS)

#define ZERO_OBJECT(p) memset(p, 0, sizeof(*(p)))


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

    // Volume ranges from 0 to 1
    float volume;

    // Pan ranges from -1 to 1
    float pan;

    // Pitch ranges from 0 to 1 (affects speed as well)
    float pitch;

    int loop_count;

} Sound_Channel;

typedef struct
{
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

    // Global mutex
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

bool ChannelStatusIsFree(int status)
{
    return status == SOUND_CHANNEL_STATUS_STOPPED || status == SOUND_CHANNEL_STATUS_UNINITIALIZED;
}

// Returns -1 on error
int Sound_FindFreeChannel()
{
    int result = -1;
    int i;

    for(i = 0; i < SOUND_CHANNEL_COUNT; i++)
    {
        Sound_Channel *c = &channels[i];
        if(ChannelStatusIsFree(c->status))
        {
            result = i;
            break;
        }
    }

    return result;
}

Sound_Channel *Sound_GetChannel(int index)
{
    if(index >= 0 && index < SOUND_CHANNEL_COUNT)
    {
        return &channels[index];
    }

    return NULL;
}


Sound_Channel *Sound_FindFreeChannelPointer(int index_in, int *index_out)
{
    Sound_Channel *result = 0;
    int index = -1;

    if(index_in == -1)
    {
        index = Sound_FindFreeChannel();
        result = Sound_GetChannel(index);
    }
    else
    {
        result = Sound_GetChannel(index_in);
        if(ChannelStatusIsFree(result->status))
        {
            index = index_in;
        }
        else
        {
            result = 0;
        }
    }

    *index_out = index;
    
    return result;
}

void data_callback2(ma_device* pDevice, void* output, const void* input, ma_uint32 frame_count)
{
    GET_SOUND_ENGINE();

    Sound_DeviceLock();

    ma_engine_read_pcm_frames(engine, output, frame_count, NULL);

    Sound_DeviceUnlock();
}

static void InitChannels()
{
    int i;
    memset(channels, 0, sizeof(channels));

    for(i = 0; i < SOUND_CHANNEL_COUNT; i++)
    {
        Sound_Channel *c = &channels[i];
        c->volume = 1;
        c->pitch = 1;
        c->pan = 0;
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
    GET_DECODER_CONFIG();

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
    sound->data = data;
    sound->size = size;

    return sound;
}

void Sound_Rewind(Sound *sound)
{
}

void cb_channel_end(void *user_data, ma_sound *handle)
{
    // Fixme!!! Mutex

    Sound_Channel *c = (Sound_Channel *)user_data;
    c->status = SOUND_CHANNEL_STATUS_STOPPED;
    printf("End channel\n");
}


bool Sound_ChannelIsFree(int index)
{
    Sound_Channel *c = Sound_GetChannel(index);
    if(c)
    {
        return c->status == SOUND_CHANNEL_STATUS_UNINITIALIZED || c->status == SOUND_CHANNEL_STATUS_STOPPED;
    }

    return false;
}

static void Sound_ChannelFreeInternal(Sound_Channel *c)
{
    ma_decoder_uninit(&c->decoder);
    ma_sound_uninit(&c->sound_handle);
}

int Channel_Stop(int channel_index)
{
    Sound_Channel *c = Sound_GetChannel(channel_index);
    if(c)
    {
        ma_sound_stop(&c->sound_handle);
    }

    c->status = SOUND_CHANNEL_STATUS_STOPPED;
}

// This function is in charge of looping
// We _do not_!!! call ma_sound_set_looping() or use MA_SOUND_FLAG_LOOPING
// Instead, we manually keep track of the loop count ourselves.
ma_data_source *cb_on_get_next(ma_data_source *data_source)
{
    bool should_loop = false;
    ma_decoder *decoder = (ma_decoder *)data_source;
    Sound_Channel *c = decoder->pUserData;
    ma_data_source *next = 0;

    if(c->loop_count > 0)
    {
        c->loop_count -=1;
    }

    should_loop = c->loop_count != 0;
    next = should_loop ? data_source : 0;

    return next;
}

int Channel_Start(int channel_index, Sound *sound, int loop_count)
{
    Sound_Channel *c = 0;
    int index_out = -1;

    Sound_DeviceLock();

    if(channel_index == -1)
    {
        c = Sound_FindFreeChannelPointer(channel_index, &index_out);
    }
    else
    {
        c = Sound_GetChannel(channel_index);

        if(c)
        {
            index_out = channel_index;
        }
    }

    if(c)
    {
        ma_uint32 flags = 0;
        ma_sound_config config;
        ma_data_source *data_source;
        GET_DECODER_CONFIG();
        GET_SOUND_ENGINE();

        Sound_ChannelFreeInternal(c);

        c->status = SOUND_CHANNEL_STATUS_PLAYING;
        c->index = channel_index;
        c->loop_count = loop_count;
        ma_decoder_init_memory(sound->data, sound->size, decoder_config, &c->decoder);

        // The decoder needs a reference back to the channel so that we can use it in cb_on_get_next()
        // This controls the looping behaviour
        c->decoder.pUserData = c; 

        flags = MA_SOUND_FLAG_STREAM;

        data_source = c->decoder.ds.pCurrent;
        config = ma_sound_config_init();
        config.pDataSource = data_source;
        config.endCallback = cb_channel_end;
        config.pEndCallbackUserData = c;
        config.flags = flags;

        CHECK(ma_sound_init_ex(engine, &config, &c->sound_handle));
        ma_sound_set_pitch(&c->sound_handle, c->pitch);
        ma_sound_set_volume(&c->sound_handle, c->volume);
        ma_sound_set_pan(&c->sound_handle, c->pan);
        ma_data_source_set_next_callback(data_source, cb_on_get_next);

        CHECK(ma_sound_start(&c->sound_handle));
    }

    Sound_DeviceUnlock();

    return index_out;
}

int Sound_Play(Sound *sound, int channel, int loop_count)
{
    return Channel_Start(channel, sound, loop_count);
}

void Channel_SetPitch(int index, float pitch)
{
    Sound_Channel *c = Sound_GetChannel(index);

    if(c)
    {
        c->pitch = pitch;
        ma_sound_set_pitch(&c->sound_handle, pitch);
    }
}

void Channel_SetVolume(int index, float volume)
{
    Sound_Channel *c = Sound_GetChannel(index);

    if(c)
    {
        c->volume = volume;
        ma_sound_set_volume(&c->sound_handle, volume);
    }
}

void Channel_SetPan(int index, float pan)
{
    Sound_Channel *c = Sound_GetChannel(index);
    if(c)
    {
        c->pan = pan;
        ma_sound_set_pan(&c->sound_handle, pan);
    }
}

int main()
{
    Sound *sound;
    int i;
    Sound_Init();
    sound = Sound_Load("test/blade.wav");
    for(i = 0; i < 1; i++)
    {
        Channel_SetPitch(1, 0.5);
        Channel_SetPan(1, 0.5);
        Channel_Start(1, sound, 5);
        ma_sleep(2000);
    }

    for(;;)
    {
        ma_sleep(1000);
    }
	
    return 0;
}
