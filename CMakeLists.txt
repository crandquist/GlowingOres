cmake_minimum_required(VERSION 3.12)
project(GlowingOres CXX)

# Set architecture for Apple Silicon
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Manually specify GLEW paths for macOS with Homebrew
set(GLEW_INCLUDE_DIRS "/opt/homebrew/include")
set(GLEW_LIBRARIES "/opt/homebrew/lib/libGLEW.dylib")

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${GLEW_INCLUDE_DIRS}
    /opt/homebrew/include  # For Apple Silicon Macs
)

# Source files
set(SOURCES
    src/shader.cpp
    src/test_shader.cpp
)

# Create test executable for shader class
add_executable(shader_test ${SOURCES})

# Define the source files for test_glowing
set(GLOWING_SOURCES
    src/shader.cpp
    src/test_glowing.cpp
)

# Create test executable for glowing effect
add_executable(test_glowing ${GLOWING_SOURCES})

# Link with required libraries
target_link_libraries(shader_test
    glfw
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
)

target_link_libraries(test_glowing
    glfw
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
)

# macOS specific settings
if(APPLE)
    target_link_libraries(shader_test
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
    
    target_link_libraries(test_glowing
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

# Output debug information
message(STATUS "OpenGL include dir: ${OPENGL_INCLUDE_DIR}")
message(STATUS "OpenGL libraries: ${OPENGL_LIBRARIES}")
message(STATUS "GLEW include dirs: ${GLEW_INCLUDE_DIRS}")
message(STATUS "GLEW libraries: ${GLEW_LIBRARIES}")