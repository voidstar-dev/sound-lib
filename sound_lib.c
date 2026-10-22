
// Loading ma_sound from memory:
// https://gist.github.com/p1xelHer0/abed4728096ea3a88af7802cbe46cf08

// Always include asserts!
#ifdef NDEBUG
#undef NDEBUG
#include <assert.h>
#define NDEBUG
#else
#include <assert.h>
#endif



#define alloc_pointer(p) p = xalloc(sizeof *p)
#define GET_DECODER_CONFIG() const ma_decoder_config *decoder_config = &sound_config.decoder_config
#define GET_SOUND_ENGINE() ma_engine *engine = &sound_config.engine;

#define SOUND_OK(call) ((call) == MA_SUCCESS)
#define SOUND_FAILED(call) ((call) != MA_SUCCESS)


#define CHECK(call) assert((call) == MA_SUCCESS)

#define ZERO_OBJECT(p) memset(p, 0, sizeof(*(p)))

#include "sound_lib.h"


Sound_Config sound_config;
Sound_Channel channels[SOUND_CHANNEL_COUNT] = {0};

static MA_INLINE float SATURATE(float x)
{
    if (x < -1) return -1;
    if (x > +1) return +1;
    return x;
}

static void *xalloc(size_t size)
{
    return calloc(size, 1);
}

void Audio_DeviceLock()
{
    ma_mutex_lock(&sound_config.mutex);
}

void Audio_DeviceUnlock()
{
    ma_mutex_unlock(&sound_config.mutex);
}

static bool ChannelIsAvailable(int status)
{
    return status == SOUND_CHANNEL_STATUS_STOPPED || status == SOUND_CHANNEL_STATUS_UNINITIALIZED;
}

// Returns -1 on error
static int Sound_FindFreeChannel()
{
    int result = -1;
    int i;

    for(i = 0; i < SOUND_CHANNEL_COUNT; i++)
    {
        Sound_Channel *c = &channels[i];
        if(ChannelIsAvailable(c->status))
        {
            result = i;
            break;
        }
    }

    return result;
}

static Sound_Channel *Sound_GetChannel(int index)
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
        if(ChannelIsAvailable(result->status))
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

static void data_callback2(ma_device* pDevice, void* output, const void* input, ma_uint32 frame_count)
{
    GET_SOUND_ENGINE();

    Audio_DeviceLock();

    ma_engine_read_pcm_frames(engine, output, frame_count, NULL);

    Audio_DeviceUnlock();
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

bool Audio_Init()
{
    int rate = 44100;
    int channels = 2;
    ma_format format = ma_format_f32;
    ma_device_config device_config;
    ma_engine_config engine_config;

    sound_config.rate      = rate;
    sound_config.channels  = channels;
    sound_config.format    = format;
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

    return true;
}

size_t get_file_size(FILE *fp)
{
    struct stat buf = {0};

    int fd = fileno(fp);
    fstat(fd, &buf);
    return buf.st_size;
}

Sound_Buffer *SoundBuffer_Alloc()
{
    Sound_Buffer *result;
    alloc_pointer(result);
    return result;
}

// It doesn't make much sense for us to duplicate the data, but I'm going for an OpenAL approach here where we
// allow the caller to free the data after it has been passed to us
Sound_Buffer *SoundBuffer_LoadFromMemory(const void *data, ma_uint32 size)
{
    Sound_Buffer *result;
    void *sound_data;

    if(!data || !size)
    {
        return NULL;
    }

    result = SoundBuffer_Alloc();
    sound_data = malloc(size);
    memcpy(sound_data, data, size);

    result->data = sound_data;
    result->size = size;
    result->allocation_type = SOUND_ALLOCATION_MALLOC;

    return result;
}

Sound_Buffer *SoundBuffer_Load(const char *filepath)
{
    FILE *fp = fopen(filepath, "rb");
    void *data = 0;
    size_t size = 0;
    Sound_Buffer *buffer;

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

    buffer = SoundBuffer_Alloc();
    buffer->data = data;
    buffer->size = size;
    buffer->allocation_type = SOUND_ALLOCATION_MALLOC;

    return buffer;
}

void SoundBuffer_Free(Sound_Buffer *buffer)
{
    int i;
    Sound_Channel *c;

    if(!buffer) return;

    Audio_DeviceLock();

    for(i = 0; i < SOUND_CHANNEL_COUNT; i++)
    {
        c = &channels[i];
        if(c->buffer == buffer)
        {
            Channel_Stop(i);
            c->buffer = NULL;
        }
    }

    switch(buffer->allocation_type)
    {
        case SOUND_ALLOCATION_MALLOC:
            free(buffer->data);
        break;

        default:
            assert(false && "Unhandled allocation in Sound_Free()");
        break;
    }

    ZERO_OBJECT(buffer);

    Audio_DeviceUnlock();
}

static void cb_channel_end(void *user_data, ma_sound *handle)
{
    // Fixme!!! Mutex
    Sound_Channel *c = (Sound_Channel *)user_data;
    c->status = SOUND_CHANNEL_STATUS_STOPPED;
}


static void Channel_FreeInternal(Sound_Channel *c)
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
        c->status = SOUND_CHANNEL_STATUS_STOPPED;
    }

}

// This function is in charge of looping
// We _do not_!!! call ma_sound_set_looping() or use MA_SOUND_FLAG_LOOPING
// Instead, we manually keep track of the loop count ourselves.
ma_data_source *cb_on_get_next(ma_data_source *data_source)
{
    ma_decoder *decoder = (ma_decoder *)data_source;
    Sound_Channel *c = decoder->pUserData;
    ma_data_source *next = 0;

    if(c->loop_count > 0)
    {
        c->loop_count -= 1;
    }

    next = c->loop_count ? data_source : 0;

    return next;
}

static int Channel_StartInternal(int channel_index, Sound_Buffer *buffer, int loop_count, Sound *sound)
{
    Sound_Channel *c = 0;
    int index_out = -1;

    Audio_DeviceLock();

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

        Channel_FreeInternal(c);

        c->status = SOUND_CHANNEL_STATUS_PLAYING;
        c->index = channel_index;
        c->loop_count = loop_count;
        CHECK(ma_decoder_init_memory(buffer->data, buffer->size, decoder_config, &c->decoder));

        // The decoder needs a reference back to the channel so that we can use it in cb_on_get_next()
        // This controls the looping behaviour
        c->decoder.pUserData = c;
        c->buffer = buffer;
        c->sound = sound;

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

    Audio_DeviceUnlock();

    return index_out;
}

// Todo: What do we do on a null sound pointer?
int Channel_Start(int channel_index, Sound_Buffer *buffer, int loop_count)
{
    return Channel_StartInternal(channel_index, buffer, loop_count, 0);
}

void Sound_Free(Sound *sound)
{
    // Fixme!!! Free the pointer for the channel as well.
    if(!sound) return;

    SoundBuffer_Free(sound->buffer);
    ZERO_OBJECT(sound);
    free(sound);
}

Sound *Sound_Load(const char *filepath)
{
    Sound *result;


    alloc_pointer(result);
    if(result)
    {
        result->buffer = SoundBuffer_Load(filepath);
        result->volume = 1;
        result->pan = 0;
        result->pitch = 1;

        if(!result->buffer)
        {
            Sound_Free(result);
            result = 0;
            goto fail;
        }
    }


    return result;

fail:
    return result;
}

#if 1
// Todo: Make this work again with a Sound struct
void Sound_Play(Sound *sound)
{
    int channel = 0;
    int loop_count = 0;
    Sound_Channel *c = Sound_FindFreeChannelPointer(-1, &channel);
    if(c)
    {
        Channel_StartInternal(channel, sound->buffer, loop_count, sound);
    }
}
#endif

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
