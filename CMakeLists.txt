cmake_minimum_required(VERSION 3.1)

# Detect whether we are the root project, or a subproject.
# If we're a subproject, we want to disable several things
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(SOUND_LIB_ROOT_PROJECT TRUE)
else()
    set(SOUND_LIB_ROOT_PROJECT FALSE)
endif()

project(sound_lib)


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

if(SOUND_LIB_ROOT_PROJECT)
    # We only need this to compile the visual test programs
    find_package(SDL2 QUIET)
endif()

# https://stackoverflow.com/questions/2152077/is-it-possible-to-get-cmake-to-build-both-a-static-and-shared-library-at-the-sam
# The library itself
# We do a 2-step process here.
# 1. Compile the objects as an object library
set(SOUND_SOURCES src/sound_lib.c src/miniaudio.c src/extras/stb_vorbis.c)
set(TEST_SOURCES test.c)
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


# Test program defines
list(APPEND TEST_DEFS SOUND_TEST_PROGRAM)

# Builds a collection of test executables that try out different file extensions
macro(build_test name)
    string(TOUPPER ${name} DEF_NAME)
    add_executable(${name} ${TEST_SOURCES})
    target_compile_definitions(${name} PRIVATE ${DEF_NAME} ${TEST_DEFS})
    target_link_libraries(${name} PRIVATE sound_lib)
    target_include_directories(${name} PRIVATE ./src)
    list(APPEND TEST_TARGETS ${name})
endmacro()


if(SOUND_LIB_ROOT_PROJECT)
    build_test(test_wav)
    build_test(test_mp3)
    build_test(test_ogg)
    build_test(test_flac)
endif()

# Some more test utilities
if(SDL2_FOUND)
    add_executable(test_mouse test_utils/test_mouse.c)
    target_include_directories(test_mouse PRIVATE ./src)
    target_link_libraries(test_mouse SDL2 sound_lib)
    target_compile_options(test_mouse PRIVATE ${SOUND_LIB_WARNINGS})
    list(APPEND TEST_TARGETS test_mouse)
endif()

# Build examples
add_subdirectory(examples_sound_lib)

# How to install
if(SOUND_LIB_ROOT_PROJECT)
    # Install targets in their respective bin directories
    install(TARGETS sound_lib ${TEST_TARGETS} DESTINATION ${BIN_DIR})
    install(DIRECTORY resources DESTINATION ${BIN_DIR})

    # Install targets in a shared bin directory for distribution
    install(TARGETS sound_lib DESTINATION sound_lib/bin_x64)
endif()
