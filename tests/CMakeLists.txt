# Various test programs.

if(SOUND_LIB_ROOT_PROJECT)
    # We only need this to compile the visual test programs
    find_package(SDL2 QUIET)
endif()

# Test program defines
list(APPEND TEST_DEFS SOUND_TEST_PROGRAM)

set(TEST_SOURCES test.c)

set(TEST_LIBS ${SOUND_LIB_OBJECTS} ${SOUND_LIB_SHARED_LIBS})

# Builds a collection of test executables that try out different file extensions
macro(build_test name)
    string(TOUPPER ${name} DEF_NAME)
    add_executable(${name} ${TEST_SOURCES})
    target_compile_definitions(${name} PRIVATE ${DEF_NAME} ${TEST_DEFS})
    target_link_libraries(${name} PRIVATE ${TEST_LIBS})
    target_include_directories(${name} PRIVATE ${SOUND_LIB_INCLUDE_DIRECTORY})
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
    add_executable(test_mouse test_mouse.c)
    target_include_directories(test_mouse PRIVATE ${SOUND_LIB_INCLUDE_DIRECTORY})
    target_link_libraries(test_mouse SDL2 ${TEST_LIBS})
    target_compile_options(test_mouse PRIVATE ${SOUND_LIB_WARNINGS})
    list(APPEND TEST_TARGETS test_mouse)
endif()

file(COPY "../resources" DESTINATION ".")
