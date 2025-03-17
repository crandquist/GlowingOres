#ifndef SIMPLE_TEXT_RENDERER_H
#define SIMPLE_TEXT_RENDERER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <iostream>

// A simplified text renderer that uses colored quads instead of actual text
// This provides a more reliable fallback for debugging
class SimpleTextRenderer {
public:
    SimpleTextRenderer(unsigned int width, unsigned int height) : width(width), height(height) {
        // Simple shader for colored quads
        const char* vertexShaderSource = 
            "#version 410 core\n"
            "layout (location = 0) in vec2 aPos;\n"
            "uniform mat4 projection;\n"
            "void main() {\n"
            "    gl_Position = projection * vec4(aPos.xy, 0.0, 1.0);\n"
            "}\n";

        const char* fragmentShaderSource = 
            "#version 410 core\n"
            "out vec4 FragColor;\n"
            "uniform vec4 color;\n"
            "void main() {\n"
            "    FragColor = color;\n"
            "}\n";

        // Compile shader program
        shader = createShaderFromSource(vertexShaderSource, fragmentShaderSource);
        
        // Set up orthographic projection
        projection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
        
        // Create a simple quad VAO for drawing indicators
        setupQuadVAO();
    }
    
    ~SimpleTextRenderer() {
        glDeleteVertexArrays(1, &quadVAO);
        glDeleteBuffers(1, &quadVBO);
        glDeleteBuffers(1, &quadEBO);
        glDeleteProgram(shader);
    }
    
    // Render a color indicator for a value (like a bar or dot)
    void renderValueIndicator(float x, float y, float width, float height, 
                             float value, float minValue, float maxValue, 
                             glm::vec4 color) {
        // Calculate how much of the bar to fill based on value
        float fillRatio = (value - minValue) / (maxValue - minValue);
        fillRatio = glm::clamp(fillRatio, 0.0f, 1.0f);
        
        float fillWidth = width * fillRatio;
        
        // Render background bar (darker version of the color)
        glm::vec4 bgColor = glm::vec4(color.r * 0.3f, color.g * 0.3f, color.b * 0.3f, color.a);
        renderQuad(x, y, width, height, bgColor);
        
        // Render filled portion
        renderQuad(x, y, fillWidth, height, color);
    }
    
    // Render a simple indicator using a colored quad
    void renderQuad(float x, float y, float width, float height, glm::vec4 color) {
        // Use our shader
        glUseProgram(shader);
        
        // Set uniform values
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, 
                           glm::value_ptr(projection));
        glUniform4fv(glGetUniformLocation(shader, "color"), 1, glm::value_ptr(color));
        
        // Update the quad vertices for this specific position and size
        float vertices[] = {
            x,         y,          // Bottom left
            x + width, y,          // Bottom right
            x + width, y + height, // Top right
            x,         y + height  // Top left
        };
        
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        
        // Draw the quad
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        // Unbind
        glBindVertexArray(0);
        glUseProgram(0);
    }
    
    // Draw a visual indicator for each value
    void renderValueDisplays(const std::vector<std::pair<std::string, float>>& values,
                             float x, float y, float spacing) {
        float currentY = y;
        
        for (const auto& value : values) {
            // For each value, render a bar showing its relative magnitude
            float minVal = 0.0f;
            float maxVal = 1.0f;
            
            // Use different colors based on the value name
            glm::vec4 color(0.5f, 0.5f, 0.5f, 1.0f);
            
            if (value.first.find("Ambient") != std::string::npos) {
                color = glm::vec4(0.2f, 0.6f, 1.0f, 1.0f); // Blue for ambient light
            } else if (value.first.find("Bloom") != std::string::npos && 
                      value.first.find("Intensity") != std::string::npos) {
                color = glm::vec4(1.0f, 0.6f, 0.2f, 1.0f); // Orange for bloom intensity
                maxVal = 5.0f; // Adjust max value for bloom intensity
            } else if (value.first.find("Bloom") != std::string::npos && 
                      value.first.find("Threshold") != std::string::npos) {
                color = glm::vec4(0.2f, 1.0f, 0.6f, 1.0f); // Green for bloom threshold
            } else if (value.first.find("Ore") != std::string::npos) {
                color = glm::vec4(1.0f, 1.0f, 0.2f, 1.0f); // Yellow for ore
            }
            
            // Render the value indicator
            renderValueIndicator(x, currentY, 200.0f, 20.0f, value.second, minVal, maxVal, color);
            
            // Move down for the next value
            currentY -= spacing;
        }
    }
    
    // Render directional indicators for changes (up/down)
    void renderDirectionIndicator(float x, float y, bool up, bool active, glm::vec4 color) {
        if (!active) return;
        
        float size = 20.0f;
        
        if (up) {
            // Draw an up arrow
            float vertices[] = {
                x,           y,             // Bottom left
                x + size,    y,             // Bottom right
                x + size/2,  y + size       // Top middle
            };
            
            glUseProgram(shader);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, 
                              glm::value_ptr(projection));
            glUniform4fv(glGetUniformLocation(shader, "color"), 1, glm::value_ptr(color));
            
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            
            // Draw triangle
            glDrawArrays(GL_TRIANGLES, 0, 3);
        } else {
            // Draw a down arrow
            float vertices[] = {
                x,           y + size,      // Top left
                x + size,    y + size,      // Top right
                x + size/2,  y              // Bottom middle
            };
            
            glUseProgram(shader);
            glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, 
                              glm::value_ptr(projection));
            glUniform4fv(glGetUniformLocation(shader, "color"), 1, glm::value_ptr(color));
            
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            
            // Draw triangle
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
        
        glBindVertexArray(0);
        glUseProgram(0);
    }

private:
    unsigned int width, height;
    unsigned int shader;
    unsigned int quadVAO, quadVBO, quadEBO;
    glm::mat4 projection;
    
    void setupQuadVAO() {
        float vertices[] = {
            0.0f, 0.0f,  // Bottom left
            1.0f, 0.0f,  // Bottom right
            1.0f, 1.0f,  // Top right
            0.0f, 1.0f   // Top left
        };
        
        unsigned int indices[] = {
            0, 1, 2,  // First triangle
            0, 2, 3   // Second triangle
        };
        
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glGenBuffers(1, &quadEBO);
        
        glBindVertexArray(quadVAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindVertexArray(0);
    }
    
    // Helper function to create a shader program from source strings
    unsigned int createShaderFromSource(const char* vertexSource, const char* fragmentSource) {
        unsigned int vertexShader, fragmentShader, shaderProgram;
        
        // Vertex shader
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
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
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
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
};

#endif // SIMPLE_TEXT_RENDERER_H