cmake_minimum_required(VERSION 3.16)
project(step2stl_example LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Step2Stl library
# Set the path to the Step2Stl library directory
set(STEP2STL_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../build/bin/Release")
set(STEP2STL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../include")

# Check if Step2Stl library exists
if(NOT EXISTS ${STEP2STL_LIBRARY_DIR}/Step2Stl.lib AND NOT EXISTS ${STEP2STL_LIBRARY_DIR}/Step2Stl.dll)
    message(FATAL_ERROR "Step2Stl library not found. Please build the Step2Stl library first.")
endif()

# Include Step2Stl header directory
include_directories(${STEP2STL_INCLUDE_DIR})

# Link directory for Step2Stl library
link_directories(${STEP2STL_LIBRARY_DIR})

# Collect source files
set(SOURCES
    step2stl_example.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link Step2Stl library
target_link_libraries(${PROJECT_NAME} Step2Stl)

# Set output directory
set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

# Copy Step2Stl.dll to the output directory (Windows only)
if(WIN32)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${STEP2STL_LIBRARY_DIR}/Step2Stl.dll
            ${OUTPUT_DIR}/Step2Stl.dll
        COMMENT "Copying Step2Stl.dll to output directory"
    )
endif()
