#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include "shader.h"

// A simple class to render text in OpenGL
class TextRenderer {
public:
    struct Character {
        unsigned int TextureID;  // ID handle of the glyph texture
        glm::ivec2   Size;       // Size of glyph
        glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
        unsigned int Advance;    // Horizontal offset to advance to next glyph
    };

    TextRenderer(unsigned int width, unsigned int height) {
        // Initialize shader
        std::string vertexShaderSource = 
            "#version 410 core\n"
            "layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
            "out vec2 TexCoords;\n"
            "uniform mat4 projection;\n"
            "void main() {\n"
            "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
            "    TexCoords = vertex.zw;\n"
            "}\n";

        std::string fragmentShaderSource = 
            "#version 410 core\n"
            "in vec2 TexCoords;\n"
            "out vec4 color;\n"
            "uniform sampler2D text;\n"
            "uniform vec3 textColor;\n"
            "void main() {\n"
            "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
            "    color = vec4(textColor, 1.0) * sampled;\n"
            "}\n";

        // Create shader
        textShader = createShaderFromSource(vertexShaderSource, fragmentShaderSource);

        // Set up projection matrix
        projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

        // Configure VAO/VBO for text quads
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Generate a simple bitmap font texture
        generateSimpleFont();
    }

    ~TextRenderer() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteProgram(textShader);
        glDeleteTextures(1, &fontTexture);
    }

    void renderText(std::string text, float x, float y, float scale, glm::vec3 color) {
        // Activate shader
        glUseProgram(textShader);
        glUniformMatrix4fv(glGetUniformLocation(textShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(textShader, "textColor"), color.x, color.y, color.z);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        // Bind font texture
        glBindTexture(GL_TEXTURE_2D, fontTexture);
        glUniform1i(glGetUniformLocation(textShader, "text"), 0);

        // Calculate character positions and render each character
        float xPos = x;
        float yPos = y;

        for (char c : text) {
            if (c == '\n') {
                // Handle newline character
                xPos = x;
                yPos -= 24.0f * scale; // Move down by character height
                continue;
            }

            // Simple fixed-width character rendering
            float charWidth = 16.0f * scale;
            float charHeight = 24.0f * scale;

            // Calculate texture coordinates for the character
            int charIndex = c - 32; // ASCII offset
            if (charIndex < 0 || charIndex >= 95) charIndex = 0; // Default to space for unknown chars

            int charsPerRow = 16;
            float texX = (charIndex % charsPerRow) / 16.0f;
            float texY = (charIndex / charsPerRow) / 6.0f;
            float texWidth = 1.0f / 16.0f;
            float texHeight = 1.0f / 6.0f;

            // Calculate vertices for the character quad
            float vertices[6][4] = {
                { xPos,              yPos + charHeight,  texX,              texY },
                { xPos,              yPos,               texX,              texY + texHeight },
                { xPos + charWidth,  yPos,               texX + texWidth,   texY + texHeight },

                { xPos,              yPos + charHeight,  texX,              texY },
                { xPos + charWidth,  yPos,               texX + texWidth,   texY + texHeight },
                { xPos + charWidth,  yPos + charHeight,  texX + texWidth,   texY }
            };

            // Update VBO with new vertices
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // Advance to next character
            xPos += charWidth;
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

private:
    unsigned int textShader;
    unsigned int VAO, VBO;
    unsigned int fontTexture;
    glm::mat4 projection;

    // Helper function to create a shader program from source strings
    unsigned int createShaderFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
        unsigned int vertexShader, fragmentShader, shaderProgram;
        
        // Vertex shader
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexShaderSource = vertexSource.c_str();
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        
        // Check for compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        
        // Fragment shader
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fragmentShaderSource = fragmentSource.c_str();
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        
        // Check for compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        
        // Shader program
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        
        // Check for linking errors
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        
        // Delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        return shaderProgram;
    }

    // Generate a simple bitmap font texture with ASCII characters
    void generateSimpleFont() {
        // In a real implementation, you would load a proper font texture
        // Here we'll create a simple placeholder texture with uniform blocks
        
        int width = 256;  // 16 characters per row
        int height = 144; // 6 rows (96 characters)
        
        unsigned char* data = new unsigned char[width * height];
        memset(data, 0, width * height); // Start with all black
        
        // Generate a simple pattern for each character slot
        for (int i = 0; i < 95; i++) { // 95 printable ASCII characters
            int x = (i % 16) * 16;
            int y = (i / 16) * 24;
            
            // Draw a simple character representation
            for (int cy = 4; cy < 20; cy++) {
                for (int cx = 4; cx < 12; cx++) {
                    if ((cy == 4 || cy == 19) || (cx == 4 || cx == 11)) {
                        // Draw outline
                        data[(y + cy) * width + (x + cx)] = 255;
                    }
                    
                    // Add some variation based on character code
                    if ((i + 32) % 3 == 0 && cx > 6 && cy > 8 && cy < 16) {
                        data[(y + cy) * width + (x + cx)] = 255;
                    }
                    else if ((i + 32) % 3 == 1 && cx < 9 && cy > 6 && cy < 14) {
                        data[(y + cy) * width + (x + cx)] = 255;
                    }
                    else if ((i + 32) % 3 == 2 && cy > 10 && cy < 18) {
                        data[(y + cy) * width + (x + cx)] = 255;
                    }
                }
            }
        }
        
        // Generate the texture
        glGenTextures(1, &fontTexture);
        glBindTexture(GL_TEXTURE_2D, fontTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Clean up
        delete[] data;
    }
};

#endif // TEXT_RENDERER_H