#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include "miniaudio.h"


#define SOUND_CHANNEL_STATUS_UNINITIALIZED 0
#define SOUND_CHANNEL_STATUS_PLAYING 1
#define SOUND_CHANNEL_STATUS_PAUSED 2
#define SOUND_CHANNEL_STATUS_STOPPED 3
#define SOUND_CHANNEL_COUNT 32

#define SOUND_ALLOCATION_NONE 0
#define SOUND_ALLOCATION_MALLOC 1

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

typedef struct
{
    ma_decoder decoder;
    ma_sound sound_handle;

    // The sound that's currently playing on this channel
    Sound_Buffer *buffer;

    // Playing status (SOUND_CHANNEL_STATUS_*)
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

} Sound_Channel;

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

bool Audio_Init();
void Audio_DeviceLock();
void Audio_DeviceUnlock();
Sound_Buffer *SoundBuffer_Load(const char *filepath);
Sound_Buffer *SoundBuffer_LoadFromMemory(const void *data, ma_uint32 size);
void SoundBuffer_Free(Sound_Buffer *sound);

int Channel_Stop(int channel_index);
int Channel_Start(int channel_index, Sound_Buffer *buffer, int loop_count);
void Channel_SetVolume(int index, float volume);
void Channel_SetPan(int index, float pan);


// Not officially part of the API
void ma_sleep(ma_uint32 milliseconds);
