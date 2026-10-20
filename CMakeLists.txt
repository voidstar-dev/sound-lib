cmake_minimum_required(VERSION 3.1)
project(sound_lib)

set(SOUND_SOURCES main.c miniaudio.c)
set(TEST_SOURCES test.c)
add_library(sound_lib SHARED ${SOUND_SOURCES})
target_link_libraries(sound_lib PRIVATE m pthread dl)
target_compile_options(sound_lib PRIVATE -Wdeclaration-after-statement -Werror)


add_executable(test_wav ${TEST_SOURCES})
target_compile_definitions(test_wav PRIVATE TEST_WAV)
target_link_libraries(test_wav PRIVATE sound_lib)


install(TARGETS sound_lib test_wav DESTINATION .)
