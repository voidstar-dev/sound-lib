# Sound Lib
A beginner-friendly audio playback API that's intended to be easy to port to other languages.

## Quick Start
All the files you need are in the `src` folder.
Simply copy them to your project source folder, and add them to your compiler sources.

Starter build scripts are located in the [scripts](./scripts) folder.

## Overview
```c
#include <stdio.h>
#include "sound_lib.h"
int main(int argc, char **argv)
{
    Sound_Buffer *buffer;
    Audio_Init();
    buffer = SoundBuffer_Load("resources/sample.wav");
    Channel_Play(0, buffer, 0);
    for(;;)
    {
        getchar();
    }
	
    return 0;
}
```

## Documentation
Located in [docs/sound_lib.html](docs/sound_lib.html)


## Examples
Located in the [examples_sound_lib](examples_sound_lib/) folder.

## Binaries
Pre-compiled binaries can be found on the [releases](releases/latest) page.


## Building with CMake
The CMake file is set up to generate a shared library for integration with other programming languages.
It also generates example programs from the `examples_sound_lib` folder.

```
cmake -S . -B build -G Ninja
ninja -C build
```

The above should work for native builds.


Cross compilation is currently supported on Linux via MinGW:

```
cmake -S . -B build_mingw -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain_mingw_x64.cmake
ninja -C build_mingw
```

## About
This library uses miniaudio as a backend, so it should be quite portable.
It has been tested on Windows and Linux.
