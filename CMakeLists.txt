cmake_minimum_required(VERSION 3.1)
project(sound_lib)

if(UNIX AND NOT APPLE)
    set(BIN_DIR bin_linux)
    set(SHARED_LIBS m pthread dl)
    set(COMMON_WARNINGS -Wdeclaration-after-statement -Werror)

endif()

if(MINGW OR WINDOWS)
    set(BIN_DIR bin_win)
endif()

set(TEST_TARGETS)


set(SOUND_SOURCES sound_lib.c miniaudio.c extras/stb_vorbis.c)
set(TEST_SOURCES test.c)
add_library(sound_lib SHARED ${SOUND_SOURCES})
target_link_libraries(sound_lib PRIVATE ${SHARED_LIBS})
target_compile_options(sound_lib PRIVATE ${COMMON_WARNINGS})




# Builds a collection of test executables that try out different file extensions
macro(build_test name)
    string(TOUPPER ${name} DEF_NAME)
    add_executable(${name} ${TEST_SOURCES})
    target_compile_definitions(${name} PRIVATE ${DEF_NAME})
    target_link_libraries(${name} PRIVATE sound_lib)
    list(APPEND TEST_TARGETS ${name})
endmacro()


build_test(test_wav)
build_test(test_mp3)
build_test(test_ogg)
build_test(test_flac)


# Some more test utilities
add_executable(test_mouse test_utils/test_mouse.c)
target_include_directories(test_mouse PRIVATE .)
target_link_libraries(test_mouse SDL2 sound_lib)
target_compile_options(test_mouse PRIVATE ${COMMON_WARNINGS})
list(APPEND TEST_TARGETS test_mouse)

install(TARGETS sound_lib ${TEST_TARGETS} DESTINATION ${BIN_DIR})
install(DIRECTORY test DESTINATION ${BIN_DIR})
