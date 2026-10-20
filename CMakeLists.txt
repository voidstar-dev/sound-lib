cmake_minimum_required(VERSION 3.1)
project(sound_lib)

set(SOUND_SOURCES main.c miniaudio.c)
add_executable(sound_lib ${SOUND_SOURCES})
target_link_libraries(sound_lib PRIVATE m pthread dl)
target_compile_options(sound_lib PRIVATE -Wdeclaration-after-statement -Werror)

install(TARGETS sound_lib DESTINATION .)
