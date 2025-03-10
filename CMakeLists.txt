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

# Create test executable for GLEW
add_executable(glew_test 
    src/test.cpp
)

# Link with required libraries
target_link_libraries(glew_test
    glfw
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
)

# macOS specific settings
if(APPLE)
    target_link_libraries(glew_test
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