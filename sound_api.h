#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>

#include "miniaudio.h"


#define SOUND_CHANNEL_STATUS_UNINITIALIZED 0
#define SOUND_CHANNEL_STATUS_PLAYING 1
#define SOUND_CHANNEL_STATUS_PAUSED 2
#define SOUND_CHANNEL_STATUS_STOPPED 3
#define SOUND_CHANNEL_COUNT 32


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

bool Audio_Init();
void Audio_DeviceLock();
void Audio_DeviceUnlock();
Sound *Sound_Load(const char *filepath);
void Sound_Free(Sound *sound);

int Channel_Stop(int channel_index);
int Channel_Start(int channel_index, Sound *sound, int loop_count);
int Sound_Play(Sound *sound, int channel, int loop_count);
void Channel_SetVolume(int index, float volume);
void Channel_SetPan(int index, float pan);


// Not officially part of the API
void ma_sleep(ma_uint32 milliseconds);
