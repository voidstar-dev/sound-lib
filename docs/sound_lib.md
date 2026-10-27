## Audio_Init
Initialise the audio library with sensible defaults. 
The current defaults are:
44100 hz, 2 channel, 32-bit floating point audio.

## Audio_InitEx
Initialise the audio library with user-provided parameters.
The format is currently hardcoded to be 32-bit floating point.

## Audio_DeviceLock
Lock the audio device. Note that locking multiple times is currently not supported, and will result in a deadlock!

## Audio_DeviceUnlock
Unlock the audio device.

## SoundBuffer_Load
Load a sound into memory from a file.

## SoundBuffer_LoadFromMemory
Load a sound into memory from other memory. Upon returning, the original data is no longer needed and can be freed if necessary.

## SoundBuffer_Free
Free the sound buffer.

## Channel_Play
Bind a buffer to the channel and play it, resampling to the default device format.
This function expects that the buffer data is in some sort of decodable container (.WAV, .MP3)...
If you want to play raw, uncompressed data, then call Channel_PlayEx().
If loop_count is negative, then the buffer will play infinitely.
Pass -1 to chanel_index to automatically find a free channel.

Returns: The channel index if successful, -1 on error.

## Channel_PlayEx
Bind a buffer to the channel and play it. If sample_rate, format, or channels are non-zero, then the library will resample the buffer
to that specification, and then again to the output device format.
If raw_audio is true, then this indicates that the buffer is an uncompressed buffer of raw audio data.
In this case, sample_rate, format, and channels _must_ be specified.
If loop_count is negative, then the buffer will play infinitely.
Pass -1 to chanel_index to automatically find a free channel.

Returns: The channel index if successful, -1 on error.

## Channel_Stop
Stop the chanel.

## Channel_SetVolume
Set the channel volume. The range is typically 0 to 1 (a normalised floating point value).
Values exceeding 1 are allowed, but not recommended.

## Channel_SetPan
Set the channel pan. Values are in the range of -1 (left) to 1 (right).

## Channel_SetPitch
Set the channel pitch. Values begin at 0.

## Channel_IsPlaying
Returns true if the channel is playing.

## Channel_GetPlayCursorMS
Get the current timestamp of the playing sound in milliseconds.
For looping sounds, this value will wrap around.

## Channel_SetPlayCursorMS
Seek to the specified time in milliseconds in the buffer being played by the channel.

## Channel_GetPlayingTimeMS
Get the total amount of time since Channel_Play / Channel_PlayEx was called

## Channel_GetPlayCursor
More user friendly name of Channel_GetPlayCursorMS. Uses the same units

## Channel_SetPlayCursor
More user friendly name of Channel_SetPlayCursorMS. Uses the same units.

## Channel_GetPlayingTime
More user friendly name of Channel_GetPlayingTimeMS. Uses the same units.

