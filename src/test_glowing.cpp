// Define STB Image implementation before including the header
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>
#include "shader.h"
#include "simple_post.h"  // Use the simplified post-processor

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float &ambientLight, int &currentOreIndex);
unsigned int loadTexture(const char* path);

// Global variables
float ambientLight = 0.5f; // Ambient light level (0.0 = dark, 1.0 = bright)
int currentOreIndex = 0;   // Current ore being displayed

// Post-processor instance (simple version for now)
SimplePostProcessor* postProcessor = nullptr;

// Struct to hold ore properties
struct OreProperties {
    std::string name;
    glm::vec3 color;
    float glowStrength;
    unsigned int diffuseMap;
    unsigned int emissiveMap;
};

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure GLFW for macOS
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Minecraft Glowing Ore Test", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    
    // Print current directory to help with debugging file paths
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    
    // Initialize simple post-processor
    try {
        postProcessor = new SimplePostProcessor(SCR_WIDTH, SCR_HEIGHT);
        std::cout << "Post-processor initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize post-processor: " << e.what() << std::endl;
        return -1;
    }
    
    // Load and compile shaders - first try the glowing shaders, fall back to basic if they don't exist
    Shader* activeShader = nullptr;
    try {
        activeShader = new Shader("../shaders/glowing.vert", "../shaders/glowing.frag");
        std::cout << "Successfully loaded glowing shaders" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load glowing shaders, falling back to basic: " << e.what() << std::endl;
        try {
            activeShader = new Shader("../shaders/basic.vert", "../shaders/basic.frag");
            std::cout << "Successfully loaded basic shaders as fallback" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load basic shaders too: " << e.what() << std::endl;
            glfwTerminate();
            return -1;
        }
    }
    
    // Set up vertex data for a Minecraft-style cube
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };
    
    // Set up VAO and VBO
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Texture coords attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // Define ore types
    std::vector<OreProperties> ores;
    
    // Diamond ore
    OreProperties diamond;
    diamond.name = "Diamond";
    diamond.color = glm::vec3(0.0f, 0.8f, 1.0f); // Light blue
    diamond.glowStrength = 2.0f;
    
    try {
        diamond.diffuseMap = loadTexture("../textures/diamond/diffuse.png");
        diamond.emissiveMap = loadTexture("../textures/diamond/emissive.png");
        ores.push_back(diamond);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load diamond textures, using fallback color: " << e.what() << std::endl;
    }
    
    // If no textures were loaded, at least add one default ore
    if (ores.empty()) {
        diamond.diffuseMap = 0; // We'll just use a solid color
        diamond.emissiveMap = 0;
        ores.push_back(diamond);
    }
    
    // Camera position
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    
    // Print instructions
    std::cout << "Controls:" << std::endl;
    std::cout << " - Up/Down arrows: Adjust ambient light level" << std::endl;
    std::cout << " - Left/Right arrows: Switch between ore types (when more are available)" << std::endl;
    
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Process input
        processInput(window, ambientLight, currentOreIndex);
        
        // Begin rendering to post-processing framebuffer
        postProcessor->beginRender();
        
        // Clear framebuffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Activate shader
        activeShader->use();
        
        // Set camera-related uniforms
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * 0.5f, glm::vec3(0.5f, 1.0f, 0.0f));
        
        // First check if the shader has these uniforms (it might be the basic shader as fallback)
        GLint modelLoc = glGetUniformLocation(activeShader->ID, "model");
        if (modelLoc != -1) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        }
        
        GLint viewLoc = glGetUniformLocation(activeShader->ID, "view");
        if (viewLoc != -1) {
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        }
        
        GLint projLoc = glGetUniformLocation(activeShader->ID, "projection");
        if (projLoc != -1) {
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        }
        
        // Set ore-specific properties and glowing parameters if the shader supports them
        GLint viewPosLoc = glGetUniformLocation(activeShader->ID, "viewPos");
        if (viewPosLoc != -1) {
            glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
        }
        
        GLint ambientLightLoc = glGetUniformLocation(activeShader->ID, "ambientLight");
        if (ambientLightLoc != -1) {
            glUniform1f(ambientLightLoc, ambientLight);
        }
        
        // Make sure we have a valid ore to render
        int oreIndex = currentOreIndex % ores.size();
        OreProperties& currentOre = ores[oreIndex];
        
        // Set ore color and glow strength if the shader supports these uniforms
        GLint oreColorLoc = glGetUniformLocation(activeShader->ID, "oreColor");
        if (oreColorLoc != -1) {
            glUniform3fv(oreColorLoc, 1, glm::value_ptr(currentOre.color));
        }
        
        GLint glowStrengthLoc = glGetUniformLocation(activeShader->ID, "glowStrength");
        if (glowStrengthLoc != -1) {
            glUniform1f(glowStrengthLoc, currentOre.glowStrength);
        }
        
        // Bind textures if the shader supports them and we have valid textures
        GLint diffuseTexLoc = glGetUniformLocation(activeShader->ID, "diffuseTexture");
        GLint emissiveTexLoc = glGetUniformLocation(activeShader->ID, "emissiveTexture");
        
        if (diffuseTexLoc != -1 && currentOre.diffuseMap != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, currentOre.diffuseMap);
            glUniform1i(diffuseTexLoc, 0);
        }
        
        if (emissiveTexLoc != -1 && currentOre.emissiveMap != 0) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, currentOre.emissiveMap);
            glUniform1i(emissiveTexLoc, 1);
        }
        
        // Draw cube
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // End rendering to framebuffer and render to screen
        postProcessor->endRender();
        postProcessor->renderToScreen();
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Display current settings
        std::cout << "\rOre: " << currentOre.name << " | Ambient Light: " << ambientLight << " (Use UP/DOWN arrows to adjust)" << std::flush;
    }
    
    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    
    delete activeShader;
    delete postProcessor;
    
    glfwTerminate();
    return 0;
}

// Process keyboard input
void processInput(GLFWwindow* window, float &ambientLight, int &currentOreIndex) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Adjust ambient light
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        ambientLight = std::min(ambientLight + 0.01f, 1.0f);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        ambientLight = std::max(ambientLight - 0.01f, 0.0f);
        
    // Change ore type
    static bool rightPressed = false;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (!rightPressed) {
            currentOreIndex++;
            rightPressed = true;
        }
    } else {
        rightPressed = false;
    }
    
    static bool leftPressed = false;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (!leftPressed) {
            currentOreIndex--;
            if (currentOreIndex < 0) currentOreIndex = 0;
            leftPressed = true;
        }
    } else {
        leftPressed = false;
    }
}

// Window resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    
    // Resize post-processor framebuffers
    if (postProcessor) {
        delete postProcessor;
        postProcessor = new SimplePostProcessor(width, height);
    }
}

// Utility function to load a texture from file with Minecraft-style pixel art settings
unsigned int loadTexture(const char* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else {
            std::cerr << "Texture format not supported: " << nrComponents << " components" << std::endl;
            stbi_image_free(data);
            throw std::runtime_error("Unsupported texture format");
        }
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        
        // Minecraft-style pixel art settings - use nearest-neighbor filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  // Changed from GL_LINEAR_MIPMAP_LINEAR
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  // Changed from GL_LINEAR
        
        // We don't need mipmaps for pixel art
        // glGenerateMipmap(GL_TEXTURE_2D);  // Commented out for pixel art
        
        stbi_image_free(data);
        
        // Check if texture is 16x16 for Minecraft style
        if (width == 16 && height == 16) {
            std::cout << "Loaded 16x16 Minecraft-style texture: " << path << std::endl;
        } else {
            std::cout << "Warning: Texture is not 16x16 pixels: " << path << " (" << width << "x" << height << ")" << std::endl;
            std::cout << "For authentic Minecraft look, textures should be exactly 16x16 pixels" << std::endl;
        }
    } else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
        throw std::runtime_error("Failed to load texture");
    }
    
    return textureID;
}