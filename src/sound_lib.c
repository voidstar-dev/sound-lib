
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


#define MA_SUCCEEDED(expr) ((expr) == MA_SUCCESS)

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

// ==================================================================
// Section: [Audio Device]
// ==================================================================
void Audio_DeviceLock(void)
{
    ma_mutex_lock(&sound_config.mutex);
}

void Audio_DeviceUnlock(void)
{
    ma_mutex_unlock(&sound_config.mutex);
}

static void cb_device_read_frames(ma_device* pDevice, void* output, const void* input, ma_uint32 frame_count)
{
    GET_SOUND_ENGINE();

    Audio_DeviceLock();

    ma_engine_read_pcm_frames(engine, output, frame_count, NULL);

    Audio_DeviceUnlock();
}


static void InitChannels();


bool Audio_InitEx(const Sound_Spec *spec)
{
    int rate = spec->rate;
    int channels = spec->channels;
    ma_format format = ma_format_f32;
    ma_device_config device_config;
    ma_engine_config engine_config;

    sound_config.rate      = rate;
    sound_config.channels  = channels;
    sound_config.format    = format;
    ma_mutex_init(&sound_config.mutex);

    // Init the config that all decoders from this point on will use.
    sound_config.decoder_config = ma_decoder_config_init(sound_config.format, sound_config.channels, sound_config.rate);

    device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format   = format;
    device_config.playback.channels = channels;
    device_config.sampleRate        = rate;
    device_config.dataCallback      = cb_device_read_frames;

    InitChannels();

    if(ma_device_init(NULL, &device_config, &sound_config.device) != MA_SUCCESS)
    {
        goto cleanup;
    }


    engine_config = ma_engine_config_init();
    engine_config.pDevice = &sound_config.device;
    engine_config.noAutoStart = MA_TRUE;

    if(ma_engine_init(&engine_config, &sound_config.engine) != MA_SUCCESS)
    {
        goto cleanup;
    }
    if(ma_engine_start(&sound_config.engine))
    {
        goto cleanup;
    }

    return true;

cleanup:
    // Fixme!!! Make sure this works!
    ma_mutex_uninit(&sound_config.mutex);
    ma_engine_uninit(&sound_config.engine);
    ma_device_uninit(&sound_config.device);
    return false;
}


bool Audio_Init(void)
{
    Sound_Spec spec = {0};

    spec.rate = 44100;
    spec.channels = 2;
    spec.format = ma_format_f32;

    return Audio_InitEx(&spec);
}

// ==================================================================
// Section: [SoundBuffer]
// ==================================================================
Sound_Buffer *SoundBuffer_Alloc()
{
    Sound_Buffer *result;
    alloc_pointer(result);
    return result;
}

// It doesn't make much sense for us to duplicate the data, but I'm going for an OpenAL approach here where we
// allow the caller to free the data after it has been passed to us
SOUNDAPI Sound_Buffer *SoundBuffer_LoadFromMemory(const void *data, ma_uint32 size)
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

static size_t get_file_size(FILE *fp)
{
    struct stat buf = {0};

    int fd = fileno(fp);
    fstat(fd, &buf);
    return buf.st_size;
}

SOUNDAPI Sound_Buffer *SoundBuffer_Load(const char *filepath)
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

SOUNDAPI void SoundBuffer_Free(Sound_Buffer *buffer)
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


// ==================================================================
// Section: [Channel]
// ==================================================================
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
        c->index = i;
    }
}

static bool ChannelIsAvailable(int status)
{
    return status == SOUND_STATUS_STOPPED || status == SOUND_STATUS_UNINITIALIZED;
}

// Returns -1 on error
static int FindFreeChannel()
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

static Sound_Channel *GetChannel(int index)
{
    if(index >= 0 && index < SOUND_CHANNEL_COUNT)
    {
        return &channels[index];
    }

    return NULL;
}


static Sound_Channel *FindFreeChannelPointer(int index_in, int *index_out)
{
    Sound_Channel *result = 0;
    int index = -1;

    if(index_in == -1)
    {
        index = FindFreeChannel();
        result = GetChannel(index);
    }
    else
    {
        result = GetChannel(index_in);
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


static void cb_channel_end(void *user_data, ma_sound *handle)
{
    // Note: This is called from the audio thread (cb_device_read_frames)
    // so we don't need to lock the audio device here again
    Sound_Channel *c = (Sound_Channel *)user_data;
    c->status = SOUND_STATUS_STOPPED;

    // Zero out this sound as well
    if(c->sound)
    {
        c->sound->channel = 0;
        c->sound->status = SOUND_STATUS_STOPPED;
        c->sound = 0;
    }
}

// This function is in charge of looping
// We _do not_!!! call ma_sound_set_looping() or use MA_SOUND_FLAG_LOOPING
// Instead, we manually keep track of the loop count ourselves.
static ma_data_source *cb_on_get_next_wrapper(ma_data_source *data_source, Sound_Channel *c)
{
    ma_data_source *next = 0;
    if(c->loop_count > 0)
    {
        c->loop_count -= 1;
    }

    next = c->loop_count ? data_source : 0;

    return next;
}

// We must obtain the data pointer in a slightly different way based on how we created this channel
static ma_data_source *cb_on_get_next_decoder(ma_data_source *data_source)
{
    ma_decoder *decoder = (ma_decoder *)data_source;
    Sound_Channel *c = decoder->pUserData;
    return cb_on_get_next_wrapper(data_source, c);
}

static ma_data_source *cb_on_get_next_audio_ref(ma_data_source *data_source)
{
    ma_audio_buffer_ref_wrapper *ref = (ma_audio_buffer_ref_wrapper *)data_source;
    Sound_Channel *c = ref->pUserData;
    return cb_on_get_next_wrapper(data_source, c);
}


static void Channel_FreeInternal(Sound_Channel *c);


static int Channel_PlayInternal(int channel_index, Sound_Buffer *buffer, int loop_count, Sound *sound, int sample_rate, ma_format format, int channels, bool raw_audio)
{
    Sound_Channel *c = 0;
    int index_out = -1;

    // Return right away on a NULL buffer.
    if(!buffer)
    {
        return -1;
    }

    Audio_DeviceLock();

    if(channel_index == -1)
    {
        c = FindFreeChannelPointer(channel_index, &index_out);
    }
    else
    {
        c = GetChannel(channel_index);

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
        ma_decoder_config local_config;

        GET_DECODER_CONFIG();
        GET_SOUND_ENGINE();

        local_config = *decoder_config;

        Channel_FreeInternal(c);

        c->status = SOUND_STATUS_PLAYING;
        c->index = channel_index;
        c->loop_count = loop_count;


        // We can manually init the decoder here with a specified sample rate, format, and channel count
        if(sample_rate)
        {
            local_config.sampleRate = sample_rate;
        }
        if(format)
        {
            local_config.format = format;
        }
        if(channels)
        {
            local_config.channels = channels;
        }

        if(raw_audio)
        {
            // Load uncompressed raw audio directly.
            // In this case, we must have all of the parameters supplied
            ma_audio_buffer_config buffer_config;
            int bytes_per_frame = ma_get_bytes_per_frame(format, channels);
            int size_in_frames = buffer->size / bytes_per_frame;
            buffer_config = ma_audio_buffer_config_init(format, channels, size_in_frames, buffer->data, 0);
            buffer_config.sampleRate = sample_rate;

            // CHECK(ma_audio_buffer_init(&buffer_config, &c->ref.ref));

            CHECK(ma_audio_buffer_ref_init(format, channels, buffer->data, size_in_frames, &c->ref.ref));
            c->ref.pUserData = c;
            c->ref.ref.sampleRate = sample_rate;
            // data_source = c->ref.ref.ref.ds.pCurrent;
            data_source = c->ref.ref.ds.pCurrent;
        }
        else
        {
            CHECK(ma_decoder_init_memory(buffer->data, buffer->size, &local_config, &c->decoder));
            c->decoder.pUserData = c;
            data_source = c->decoder.ds.pCurrent;
        }

        c->buffer = buffer;
        c->sound = sound;
        flags = MA_SOUND_FLAG_STREAM;

        config = ma_sound_config_init();
        config.pDataSource = data_source;
        config.endCallback = cb_channel_end;
        config.pEndCallbackUserData = c;
        config.flags = flags;

        CHECK(ma_sound_init_ex(engine, &config, &c->sound_handle));
        ma_sound_set_pitch(&c->sound_handle, c->pitch);
        ma_sound_set_volume(&c->sound_handle, c->volume);
        ma_sound_set_pan(&c->sound_handle, c->pan);

        // Now we need to check what callback type to set
        if(raw_audio)
        {
            ma_data_source_set_next_callback(data_source, cb_on_get_next_audio_ref);
        }
        else
        {
            ma_data_source_set_next_callback(data_source, cb_on_get_next_decoder);
        }

        if(c->sound)
        {
            c->sound->status = SOUND_STATUS_PLAYING;
        }

        c->use_raw_audio = raw_audio;

        CHECK(ma_sound_start(&c->sound_handle));
    }

    Audio_DeviceUnlock();

    return index_out;
}

// Todo: What do we do on a null sound pointer?
SOUNDAPI int Channel_Play(int channel_index, Sound_Buffer *buffer, int loop_count)
{
    return Channel_PlayInternal(channel_index, buffer, loop_count, 0, 0, 0, 0, 0);
}

SOUNDAPI int Channel_PlayEx(int channel_index, Sound_Buffer *buffer, int loop_count, int sample_rate, int channels, ma_format format, bool raw_audio)
{
    return Channel_PlayInternal(channel_index, buffer, loop_count, 0, sample_rate, format, channels, raw_audio);
}

static void Channel_FreeInternal(Sound_Channel *c)
{
    ma_decoder_uninit(&c->decoder);
    ma_sound_uninit(&c->sound_handle);
    ma_audio_buffer_ref_uninit(&c->ref.ref);
    c->ref.pUserData = 0;
}

SOUNDAPI void Channel_Stop(int channel_index)
{
    Sound_Channel *c = GetChannel(channel_index);
    if(c)
    {
        ma_sound_stop(&c->sound_handle);
        c->status = SOUND_STATUS_STOPPED;
    }
}


SOUNDAPI void Channel_SetPitch(int index, float pitch)
{
    Sound_Channel *c = GetChannel(index);

    if(c)
    {
        c->pitch = pitch;
        ma_sound_set_pitch(&c->sound_handle, pitch);
    }
}

SOUNDAPI void Channel_SetVolume(int index, float volume)
{
    Sound_Channel *c = GetChannel(index);

    if(c)
    {
        c->volume = volume;
        ma_sound_set_volume(&c->sound_handle, volume);
    }
}

SOUNDAPI void Channel_SetPan(int index, float pan)
{
    Sound_Channel *c = GetChannel(index);
    if(c)
    {
        c->pan = pan;
        ma_sound_set_pan(&c->sound_handle, pan);
    }
}

SOUNDAPI bool Channel_IsPlaying(int index)
{
    Sound_Channel *c = 0;
    bool result = false;

    Audio_DeviceLock();

    c = GetChannel(index);
    if(c)
    {
        result = c->status == SOUND_STATUS_PLAYING;
    }

    Audio_DeviceUnlock();

    return result;

}

#if 0
ma_data_source *GetChannelDataSource(Sound_Channel *c)
{
    if(c->use_raw_audio)
    {
        return c->ref.ref.ds.pCurrent;
    }
    else
    {
        return c->decoder.ds.pCurrent;
    }
}
#endif

SOUNDAPI float Channel_GetPlayCursorMS(int channel_index)
{
    float result = -1;
    Sound_Channel *c;

    Audio_DeviceLock();

    c = GetChannel(channel_index);
    if(c)
    {
        ma_sound_get_cursor_in_seconds(&c->sound_handle, &result);
        result *= 1000;
    }

    Audio_DeviceUnlock();

    return result;
}

SOUNDAPI float Channel_GetPlayingTimeMS(int channel_index)
{
    float result = -1;
    Sound_Channel *c;

    Audio_DeviceLock();

    c = GetChannel(channel_index);
    if(c)
    {
        result = ma_sound_get_time_in_milliseconds(&c->sound_handle);
    }

    Audio_DeviceUnlock();

    return result;
}

SOUNDAPI bool Channel_SetPlayCursorMS(int channel_index, float milliseconds)
{
    Sound_Channel *c;
    bool result = false;

    Audio_DeviceLock();

    c = GetChannel(channel_index);
    if(c)
    {
        float seconds = milliseconds / 1000;
        result = MA_SUCCEEDED(ma_sound_seek_to_second(&c->sound_handle, seconds));
    }

    Audio_DeviceUnlock();

    return result;
}

SOUNDAPI float Channel_GetPlayCursor(int channel_index)
{
    return Channel_GetPlayCursorMS(channel_index);
}

SOUNDAPI bool Channel_SetPlayCursor(int channel_index, float milliseconds)
{
    return Channel_SetPlayCursorMS(channel_index, milliseconds);
}

SOUNDAPI float Channel_GetPlayingTime(int channel_index)
{
    return Channel_GetPlayingTimeMS(channel_index);
}

// ==================================================================
// Section: [Sound]
// ==================================================================

// Todo: Make this work again with a Sound struct
SOUNDAPI void Sound_Play(Sound *sound)
{
    int channel = 0;
    int loop_count = 0;
    Sound_Channel *c = FindFreeChannelPointer(-1, &channel);
    if(c)
    {
        Channel_SetVolume(channel, sound->volume);
        Channel_SetPitch(channel, sound->pitch);
        Channel_SetPan(channel, sound->pan);
        Channel_PlayInternal(channel, sound->buffer, loop_count, sound, 0, 0, 0, 0);
    }
}

SOUNDAPI void Sound_Stop(Sound *sound)
{
    if(!sound) return;

    if(sound->channel)
    {
        // Make sure our channel is the correct one
        assert(sound->channel->sound == sound);

        // cb_channel_end will set the required data for this sound, and unlink it from the current channel
        Channel_Stop(sound->channel->index);
    }

    // In case the calllback did not fire, we set the status here anyway
    sound->status = SOUND_STATUS_STOPPED;
}

SOUNDAPI void Sound_Free(Sound *sound)
{
    // Fixme!!! Free the pointer for the channel as well.
    if(!sound) return;

    if(sound->channel)
    {
        Sound_Stop(sound);
    }

    SoundBuffer_Free(sound->buffer);
    ZERO_OBJECT(sound);
    free(sound);
}

SOUNDAPI Sound *Sound_Load(const char *filepath)
{
    Sound *result = 0;

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
