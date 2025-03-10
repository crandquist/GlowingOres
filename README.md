# Glowing Ores Shader for Minecraft and Standalone Rendering

## Project Overview

This project implements a custom GLSL shader that creates glowing effects on Minecraft ore blocks. The shader works both as a Minecraft shader pack and as a standalone OpenGL application. The implementation focuses on emissive lighting effects that make ores appear to glow in dark environments, with dynamic intensity adjustments based on ambient lighting conditions.

## Features

- Emissive lighting shader for different ore types (diamond, emerald, redstone, gold, iron, copper)
- Screen-space effects for bloom and light diffusion
- Dynamic light intensity that adjust based on environmental lighting
- Standalone OpenGL application for testing and demonstration
- Cross-platform compatibility with a focus on macOS support

## Technical Implementation

The project uses modern OpenGL techniques and is built with:

- GLEW for OpenGL extension loading
- GLFW for window creation and input handling
- GLM for mathematics operations
- GLSL 4.1 for shader programming
- CMake for build system management

The shader implementation includes:

- Fragment shaders to calculate dynamic emissive properties
- Light attenuation techniques for realistic glow effects
- Post-processing effects for bloom rendering

## Development Environment

This project was developed on macOS using Apple Silicon (M1) hardware. The OpenGL context runs on top of Apple's Metal framework (version 4.1), which introduces some compatibility requirements:

- GLSL shaders use version 410 core
- Shader attributes use modern GLSL syntax with `in`/`out` qualifiers
- Specialized error handling for Metal translation layer

## Project Structure

```bash
GlowingOres/
├── CMakeLists.txt        # Build configuration
├── include/              # Header files
│   ├── shader.h          # Shader class for GLSL management
│   └── other headers     # Additional utility headers
├── src/                  # Source files
│   ├── main.cpp          # Main application entry point
│   ├── shader.cpp        # Shader class implementation
│   └── other sources     # Additional implementation files
└── shaders/              # GLSL shader files
    ├── basic.vert        # Vertex shader
    ├── basic.frag        # Fragment shader
    └── other shaders     # Additional specialized shaders
```

## Building the Project

### Prerequisites

- CMake (3.12) or higher
- GLFW3
- GLEW
- GLM

### Build Instructions

1. Clone the repository:

    ```bash
    git clone https://github.com/yourusername/GlowingOres.git
    cd GlowingOres
    ```

2. Create a build directory:

    ```bash
    mkdir build
    cd build
    ```

3. Configure the project with CMake:

    ```bash
    cmake -DCMAKE_OSX_ARCHITECTURES=64 .. # For Apple Silicon
    ```

    or

    ```bash
    cmake .. # For other platforms
    ```

4. Build the project:

   ```bash
   make
   ```

5. Run the standalone application:

    ```bash
    ./shader_test
    ```

### Using as a Minecraft Shader

1. Install OptiFine or Iris+Sodium for Minecraft
2. Copy the `minecraft_shaders` directory to your `.minecraft/shaderspacks`
3. Rename it to something like "GlowingOres_v1.0"
4. In Minecraft, go to Options → Video Settings → Shaders and select the shader pack

## Debugging and Troubleshooting

The shader implementation includes comprehensive error checking and reporting. Common issues include:

- Shader compilation errors: Check console output for GLSL syntax issues
- OpenGL compatibility: Ensure your graphics driver supports OpenGL 3.3 or higher
- macOS specific issues: May require specific shader version (4.1) and syntax

## Future Improvements

- Additional ore types and customization options
- Performance optimizations for large-scale rendering
- Support for more complex lighting models
- Integration with Minecraft biome colors and time-of-day lighting

## License

This project is licensed under the [MIT License](LICENSE) - see the LICENSE file for details.

## Acknowledgments

This project was developed as a final project for CS 457/557 Computer Graphics Shaders class. Special thanks to Professor Mike Bailey.
