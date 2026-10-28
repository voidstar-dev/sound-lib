cmake_minimum_required(VERSION 3.1)

# Detect whether we are the root project, or a subproject.
# If we're a subproject, we want to disable several things
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(SOUND_LIB_ROOT_PROJECT TRUE)
else()
    set(SOUND_LIB_ROOT_PROJECT FALSE)
endif()

project(sound_lib)

set(SOUND_LIB_INCLUDE_DIRECTORY ${CMAKE_SOURCE_DIR}/src)


if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(LINUX)
    set(SOUND_LIB_SHARED_LIBS m pthread dl)
endif()

if(MINGW OR LINUX)
    list(APPEND SOUND_LIB_WARNINGS -Wdeclaration-after-statement -Wreturn-type -Wattributes -Werror)
endif()

if(MINGW)
    # We want a static version of gcc to avoid including extra dlls in the distribution
    list(APPEND SOUND_LIB_CFLAGS -static-libgcc)
    list(APPEND SOUND_LIB_LDFLAGS -static-libgcc)
endif()

# Installation directories
# Not entirely correct, we may be building 32-bits

if(MINGW)
    set(BIN_DIR bin_mingw_x64)
endif()

if(LINUX)
    set(BIN_DIR bin_linux_x64)
endif()

if(MSVC)
    set(BIN_DIR bin_msvc_x64)
endif()


# https://stackoverflow.com/questions/2152077/is-it-possible-to-get-cmake-to-build-both-a-static-and-shared-library-at-the-sam
# The library itself
# We do a 2-step process here.
# 1. Compile the objects as an object library
set(SOUND_SOURCES src/sound_lib.c src/miniaudio.c src/extras/stb_vorbis.c)
add_library(sound_lib_objs OBJECT ${SOUND_SOURCES})
set_property(TARGET sound_lib_objs PROPERTY POSITION_INDEPENDENT_CODE 1)

# 2. Link them into a shared library.
set(SOUND_LIB_OBJECTS $<TARGET_OBJECTS:sound_lib_objs>)
add_library(sound_lib SHARED ${SOUND_LIB_OBJECTS})
target_link_libraries(sound_lib PRIVATE ${SOUND_LIB_SHARED_LIBS})
target_compile_options(sound_lib PRIVATE ${SOUND_LIB_WARNINGS} ${SOUND_LIB_CFLAGS})
target_link_options(sound_lib PRIVATE ${SOUND_LIB_LDFLAGS})

# Rationale: We can re-use the object files when compling the examples.
# This way, the example programs don't depend on a shared library being around.

# If we're using mingw, keep to the Windows library conventions
# https://stackoverflow.com/questions/31038963/how-do-you-rename-a-library-filename-in-cmake
# https://cmake.org/cmake/help/latest/prop_tgt/IMPORT_PREFIX.html
if(MINGW)
    set_target_properties(sound_lib PROPERTIES PREFIX "")
    set_target_properties(sound_lib PROPERTIES IMPORT_PREFIX "")
    set_target_properties(sound_lib PROPERTIES IMPORT_SUFFIX ".lib")
endif()


# Build examples
add_subdirectory(examples_sound_lib)

# Build tests
add_subdirectory(tests)

# How to install
if(SOUND_LIB_ROOT_PROJECT)
    # Install targets in their respective bin directories
    install(TARGETS sound_lib DESTINATION ${BIN_DIR})

    # Install targets in a shared bin directory for distribution
    install(TARGETS sound_lib DESTINATION sound_lib/bin_x64)
endif()
