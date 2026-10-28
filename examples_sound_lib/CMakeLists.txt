set(LIB_SRCS 
    ../src/miniaudio.c
    ../src/miniaudio.c
    ../src/sound_lib.c
    ../src/extras/stb_vorbis.c
)

# Use the root project variables for these examples
# In this case, we're not linking this library dynamically.
set(EXAMPLE_SHARED_LIBS ${SOUND_LIB_SHARED_LIBS})
set(INCLUDE_DIR ../src)

add_executable(playback ${LIB_SRCS} playback.c)
target_include_directories(playback PRIVATE ${INCLUDE_DIR})
target_link_libraries(playback PRIVATE ${EXAMPLE_SHARED_LIBS})


# Copy the resource files directly to the build folder.
# We don't want to install these examples.
file(COPY "../resources" DESTINATION ".")
