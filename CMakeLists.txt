cmake_minimum_required(VERSION 3.1)
project(sound_lib)

if(UNIX)
    set(BIN_DIR bin_linux)
    set(SHARED_LIBS m pthread dl)
endif()

if(MINGW OR WINDOWS)
    set(BIN_DIR bin_win)
endif()

set(TEST_TARGETS)

set(SOUND_SOURCES main.c miniaudio.c extras/stb_vorbis.c)
set(TEST_SOURCES test.c)
add_library(sound_lib SHARED ${SOUND_SOURCES})
target_link_libraries(sound_lib PRIVATE ${SHARED_LIBS})
target_compile_options(sound_lib PRIVATE -Wdeclaration-after-statement -Werror)

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

install(TARGETS sound_lib ${TEST_TARGETS} DESTINATION ${BIN_DIR})
install(DIRECTORY test DESTINATION ${BIN_DIR})
