cmake_minimum_required(VERSION 3.1)

# Detect whether we are the root project, or a subproject.
# If we're a subproject, we want to disable several things
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(SOUND_ROOT_PROJECT TRUE)
else()
    set(SOUND_ROOT_PROJECT FALSE)
endif()

project(sound_lib)


if(UNIX AND NOT APPLE)
    set(SHARED_LIBS m pthread dl)
    set(LINUX TRUE)
endif()

if(MINGW OR LINUX)
    list(APPEND COMMON_WARNINGS -Wdeclaration-after-statement -Wreturn-type -Wattributes -Werror)
endif()

if(MINGW)
    # We want a static version of gcc to avoid including extra dlls in the distribution
    list(APPEND WIN32_CFLAGS -static-libgcc)
    list(APPEND WIN32_LINKER_FLAGS -static-libgcc)
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

if(SOUND_ROOT_PROJECT)
    # We only need this to compile the visual test programs
    find_package(SDL2 QUIET)
endif()

# The library itself
set(SOUND_SOURCES sound_lib.c miniaudio.c extras/stb_vorbis.c)
set(TEST_SOURCES test.c)
add_library(sound_lib SHARED ${SOUND_SOURCES})
target_link_libraries(sound_lib PRIVATE ${SHARED_LIBS})
target_compile_options(sound_lib PRIVATE ${COMMON_WARNINGS} ${WIN32_CFLAGS})
target_link_options(sound_lib PRIVATE ${WIN32_LINKER_FLAGS})

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
    list(APPEND TEST_TARGETS ${name})
endmacro()


if(SOUND_ROOT_PROJECT)
    build_test(test_wav)
    build_test(test_mp3)
    build_test(test_ogg)
    build_test(test_flac)
endif()

# Some more test utilities
if(SDL2_FOUND)
    add_executable(test_mouse test_utils/test_mouse.c)
    target_include_directories(test_mouse PRIVATE .)
    target_link_libraries(test_mouse SDL2 sound_lib)
    target_compile_options(test_mouse PRIVATE ${COMMON_WARNINGS})
    list(APPEND TEST_TARGETS test_mouse)
endif()

# How to install
if(SOUND_ROOT_PROJECT)
    # Install targets in their respective bin directories
    install(TARGETS sound_lib ${TEST_TARGETS} DESTINATION ${BIN_DIR})
    install(DIRECTORY test DESTINATION ${BIN_DIR})

    # Install targets in a shared bin directory for distribution
    install(TARGETS sound_lib DESTINATION sound_lib/bin_x64)
endif()
