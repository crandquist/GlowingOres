# Minimum required CMake version
cmake_minimum_required(VERSION 3.12)

# Project name and language
project(GlowingOres CXX)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Include directories
include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    /usr/local/include  # For Intel Macs
    /opt/homebrew/include  # For Apple Silicon Macs
)

# Create a list of our source files
set(SOURCES 
    src/glad.c
    src/main.cpp
    src/shader.cpp
)

# Define the executable target
add_executable(GlowingOres ${SOURCES})

# Link libraries to our executable
target_link_libraries(GlowingOres
    ${OPENGL_LIBRARIES}
    glfw
)

# macOS specific flags
if(APPLE)
    target_compile_options(GlowingOres PRIVATE "-Wall")
    target_link_libraries(GlowingOres
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()