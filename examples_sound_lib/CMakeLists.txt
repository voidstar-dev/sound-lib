# Examples for sound_lib
# Intended to be built from the root sound_lib folder as a subdirectory project.

set(LIB_SOURCES ${SOUND_LIB_OBJECTS})

# Use the root project variables for these examples
# We're also linking against this library statically with an object library
set(EXAMPLE_SHARED_LIBS ${SOUND_LIB_SHARED_LIBS})
set(INCLUDE_DIR ../src)


# For each .c file in this directory...
file(GLOB EXAMPLE_SOURCES *.c)

foreach(example_source ${EXAMPLE_SOURCES})
    # Get the base file name
    get_filename_component(example_name ${example_source} NAME)

    # Replace the extension
    string(REPLACE ".c" "" example_name ${example_name})

    # Configure the target
    add_executable(${example_name} ${example_source} ${LIB_SOURCES})
    target_include_directories(${example_name} PRIVATE ${INCLUDE_DIR})
    target_link_libraries(${example_name} PRIVATE ${EXAMPLE_SHARED_LIBS})
endforeach()


# Copy the resource files directly to the build folder.
# We don't want to install these examples.
file(COPY "../resources" DESTINATION ".")
