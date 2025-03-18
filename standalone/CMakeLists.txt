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

# Source files for shader test
set(SHADER_TEST_SOURCES
    src/shader.cpp
    src/test_shader.cpp
)

# Source files for glowing effect with simplified post-processing
set(GLOWING_SOURCES
    src/shader.cpp
    src/post_processor.cpp  # Changed from simple_post.cpp to post_processor.cpp
    src/test_glowing.cpp
)

# Create test executable for shader class
add_executable(shader_test ${SHADER_TEST_SOURCES})

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

# Create the shader and texture directories in the build directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/textures)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/textures/diamond)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/textures/emerald)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/textures/redstone)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/textures/gold)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/textures/iron)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/textures/lapis)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/textures/copper)

# Copy shader files to build directory
file(GLOB SHADER_FILES ${CMAKE_SOURCE_DIR}/shaders/*.vert ${CMAKE_SOURCE_DIR}/shaders/*.frag)
foreach(SHADER_FILE ${SHADER_FILES})
    file(COPY ${SHADER_FILE} DESTINATION ${CMAKE_BINARY_DIR}/shaders/)
endforeach()

# Copy texture files to build directory
file(GLOB_RECURSE TEXTURE_FILES 
    ${CMAKE_SOURCE_DIR}/textures/*.png 
    ${CMAKE_SOURCE_DIR}/textures/*.jpg
)
foreach(TEXTURE_FILE ${TEXTURE_FILES})
    # Get the relative path to maintain directory structure
    file(RELATIVE_PATH REL_PATH ${CMAKE_SOURCE_DIR} ${TEXTURE_FILE})
    # Create the destination path
    get_filename_component(DEST_PATH ${CMAKE_BINARY_DIR}/${REL_PATH} DIRECTORY)
    # Create directory if it doesn't exist
    file(MAKE_DIRECTORY ${DEST_PATH})
    # Copy the file
    file(COPY ${TEXTURE_FILE} DESTINATION ${DEST_PATH})
endforeach()