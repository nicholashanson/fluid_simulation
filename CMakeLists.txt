
cmake_minimum_required(VERSION 3.10)

project(FluidSimulation VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SOURCES
    src/main.cpp
    src/grid_renderer.cpp
    src/glad.c
    src/lbm_global_data.cpp
    src/lga.cpp
    src/shader.cpp
    src/fs.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/inline
)

add_library(my_objects OBJECT ${SOURCES})

set(OBJECT_OUTPUT_DIR ${CMAKE_BINARY_DIR}/obj)

set_target_properties(my_objects PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OBJECT_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${OBJECT_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${OBJECT_OUTPUT_DIR}
)

add_custom_target(print_object_files
    COMMAND echo "Object files: $<TARGET_OBJECTS:my_objects>"
)
