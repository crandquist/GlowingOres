# Add to your existing CMakeLists.txt
add_executable(test_glowing src/shader.cpp src/test_glowing.cpp src/stb_image_imp.cpp)

target_link_libraries(test_glowing
    glfw
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
)

# macOS specific settings
if(APPLE)
    target_link_libraries(test_glowing
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()