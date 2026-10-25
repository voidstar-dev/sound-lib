#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "miniaudio.h"


#define SOUND_STATUS_UNINITIALIZED 0
#define SOUND_STATUS_PLAYING 1
#define SOUND_STATUS_PAUSED 2
#define SOUND_STATUS_STOPPED 3
#define SOUND_CHANNEL_COUNT 32

#define SOUND_ALLOCATION_NONE 0
#define SOUND_ALLOCATION_MALLOC 1

#ifdef _WIN32
#define SOUNDAPI __declspec(dllexport)
#endif

#ifndef SOUNDAPI
#define SOUNDAPI
#endif

typedef struct
{
    // Private data, do not touch!

    // Raw data of the file
    void *data;

    // The size of the file
    size_t size;

    // How this data was allocated
    int allocation_type;

} Sound_Buffer;

typedef struct Sound Sound;

// An extra wrapper to include a pointer to user data, which we need to implement looping
typedef struct
{
    ma_audio_buffer_ref ref; // Raw audio data
    void *pUserData;
} ma_audio_buffer_ref_wrapper;

typedef struct
{
    ma_sound sound_handle;


    // We support either loading through a decoder, or streaming raw, uncompressed audio data.
    // Note: These should really be in a union
    // However, for easier uninitialization, we keep these separate so that a zeroed out struct is valid
    ma_decoder decoder;
    ma_audio_buffer_ref_wrapper ref;


    // The buffer that's currently playing on this channel
    Sound_Buffer *buffer;

    // The sound that's currently playing on this channel
    Sound *sound;

    // Playing status (SOUND_STATUS_*)
    int status;

    int loop_count;
    int max_loops;

    // A reference to its own index
    int index;

    // Volume ranges from 0 to 1
    float volume;

    // Pan ranges from -1 to 1
    float pan;

    // Pitch ranges from 0 to 1 (affects speed as well)
    float pitch;

    // Use ma_audio_buffer_ref as an audio source rather than the decoder.
    bool use_raw_audio;

} Sound_Channel;


typedef struct Sound
{
    Sound_Channel *channel;
    Sound_Buffer *buffer;

    float volume;
    float pan;
    float pitch;

    // SOUND_STATUS_*
    int status;

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

SOUNDAPI bool Audio_Init();
SOUNDAPI void Audio_DeviceLock();
SOUNDAPI void Audio_DeviceUnlock();
SOUNDAPI Sound_Buffer *SoundBuffer_Load(const char *filepath);
SOUNDAPI Sound_Buffer *SoundBuffer_LoadFromMemory(const void *data, ma_uint32 size);
SOUNDAPI void SoundBuffer_Free(Sound_Buffer *sound);

SOUNDAPI int Channel_Play(int channel_index, Sound_Buffer *buffer, int loop_count);
SOUNDAPI int Channel_PlayEx(int channel_index, Sound_Buffer *buffer, int loop_count, int sample_rate, ma_format format, int channels, bool raw_audio);
SOUNDAPI int Channel_Stop(int channel_index);
SOUNDAPI void Channel_SetVolume(int index, float volume);
SOUNDAPI void Channel_SetPan(int index, float pan);
SOUNDAPI void Channel_SetPitch(int index, float pitch);
SOUNDAPI bool Channel_IsPlaying(int index);


// Incomplete for now, do not use!
SOUNDAPI Sound *Sound_Load(const char *filepath);
SOUNDAPI void Sound_Play(Sound *sound);
SOUNDAPI void Sound_Stop(Sound *sound);

#ifdef SOUND_TEST_PROGRAM

// Not officially part of the API, but still needed for the test programs.
SOUNDAPI void ma_sleep(ma_uint32 milliseconds);

#endif
