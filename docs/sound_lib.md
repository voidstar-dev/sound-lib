## Audio_Init
Initialise the audio library with sensible defaults. 
The current defaults are:
44100 hz, 2 channel, 32-bit floating point audio.

### Returns
true on success, or false on failure.

### See Also
Audio_InitEx

## Audio_InitEx
Initialise the audio library with user-provided parameters.
The format is currently hardcoded to be 32-bit floating point.

### Returns
true on success, or false on failure.

### See Also
Audio_Init
Sound_Spec



## Audio_DeviceLock
Lock the audio device mutex. 
Note that locking on the same thread multiple times while the lock is already held is currently not supported, and will result in a deadlock!
The re-entrant behaviour of a lock is called a recursive mutex.

### See Also
Audio_DeviceUnlock

## Audio_DeviceUnlock
Unlock the audio device mutex.

### See Also
Audio_DeviceLock


## SoundBuffer_Load
Load a sound into memory from a file.
Free the buffer with SoundBuffer_Free.

### Returns
A valid pointer to a Sound_Buffer, or NULL on failure.

### See Also
SoundBuffer_LoadFromMemory
SoundBuffer_Free
Sound_Buffer

## SoundBuffer_LoadFromMemory
Load a sound into memory from other memory.
Upon returning, the original data is no longer needed and can be freed if necessary.
Free the buffer with SoundBuffer_Free.

### Returns
A valid pointer to a Sound_Buffer, or NULL on failure.

### See Also
SoundBuffer_Load
SoundBuffer_Free
Sound_Buffer

## SoundBuffer_Free
Free the sound buffer.
This function will also stop any channels that were using this buffer.

### See Also
SoundBuffer_Load
SoundBuffer_LoadFromMemory

## Channel_Play
Bind a buffer to the channel and play it, resampling to the default device format.
This function expects that the buffer data is in some sort of decodable container (.WAV, .MP3, ...).
If you want to play raw, uncompressed data, then call Channel_PlayEx() with `raw_audio` set to `true`.
If `loop_count` is negative, then the buffer will loop infinitely.
Pass -1 as the `channel_index` to automatically find a free channel.

### Returns
The channel index on success, or -1 on failure.

### See Also
Channel_PlayEx
Channel_Stop
Channel_IsPlaying

## Channel_PlayEx
Bind a buffer to the channel and play it, resampling to the default device format.
If any combination of sample_rate, channels, or format are non-zero, then the library will resample the buffer
to that specification, and then again to the output device format. Otherwise, the function behaves identically to Channel_Play.
If `raw_audio` is true, then this indicates that the buffer contains uncompressed raw audio data.
In this case, `sample_rate`, `channels`, and `format` _must_ be specified.
If `loop_count` is negative, then the buffer will loop infinitely.
Pass -1 as the `channel_index` to automatically find a free channel.

### Returns
The channel index on success, or -1 on failure.

### See Also
Channel_Play
Channel_Stop
Channel_IsPlaying

## Channel_Stop
Stop the channel.

### See Also
Channel_Play
Channel_PlayEx

## Channel_SetVolume
Set the channel volume.
The range is typically 0 to 1 (a normalised floating point value).
Values exceeding 1 are allowed, but not recommended.

### See Also
Channel_SetPan
Channel_SetPitch

## Channel_SetPan
Set the channel pan. Values are in the range of -1 (left) to 1 (right).

### See Also
Channel_SetVolume
Channel_SetPitch

## Channel_SetPitch
Set the channel pitch. Values begin at 0.

### See Also
Channel_SetVolume
Channel_SetPan

## Channel_IsPlaying
Returns true if the channel is playing.

### Returns
True if the channel is playing, or false otherwise.

### See Also
Channel_Play
Channel_PlayEx

## Channel_GetPlayCursorMS
Get the current timestamp of the playing sound in milliseconds.
For looping sounds, this value will wrap around.

### Returns
The current play cursor of the channel's buffer in milliseconds, or -1 on failure.

### See Also
Channel_GetPlayCursor

## Channel_SetPlayCursorMS
Seek to the specified time (milliseconds) in the buffer being played by the channel.

### Returns
True on success, or false on failure.

### See Also
Channel_SetPlayCursor

## Channel_GetPlayingTimeMS
Get the total amount of time (milliseconds) since Channel_Play() / Channel_PlayEx() was called.

### Returns
The total amount of milliseconds since Channel_Play() / Channel_PlayEx() was called, or -1 on failure.

### See Also
Channel_GetPlayingTime


## Channel_GetPlayCursor
Get the current timestamp of the playing sound in milliseconds.
For looping sounds, this value will wrap around.
More user-friendly name for Channel_GetPlayCursorMS.

### Returns
The current play cursor of the channel's buffer in milliseconds, or -1 on failure.

### See Also
Channel_GetPlayCursorMS

## Channel_SetPlayCursor
Seek to the specified time (milliseconds) in the buffer being played by the channel.
More user-friendly name for Channel_SetPlayCursorMS.

### Returns
True on success, or false on failure.

### See Also
Channel_SetPlayCursorMS

## Channel_GetPlayingTime
Get the total amount of time (milliseconds) since Channel_Play() / Channel_PlayEx() was called.
More user-friendly name for Channel_GetPlayingTimeMS.

### Returns
The total amount of milliseconds since Channel_Play() / Channel_PlayEx() was called, or -1 on failure.

### See Also
Channel_GetPlayingTimeMS
