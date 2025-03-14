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
#include "post_processor.h"  // Added for bloom effect

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float &ambientLight, int &currentOreIndex, 
                 bool &bloomEnabled, float &bloomIntensity, float &bloomThreshold);
unsigned int loadTexture(const char* path);

// Global variables
float ambientLight = 0.5f;      // Ambient light level (0.0 = dark, 1.0 = bright)
int currentOreIndex = 0;        // Current ore being displayed

// Bloom settings
bool bloomEnabled = true;       // Whether bloom effect is enabled
float bloomThreshold = 0.5f;    // How bright a pixel needs to be to start glowing
float bloomIntensity = 1.0f;    // How strong the bloom effect is
int bloomBlurPasses = 5;        // How many times to blur (more = smoother but slower)

// Post-processor instance
PostProcessor* postProcessor = nullptr;

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
    
    // Initialize post-processor for bloom effect
    postProcessor = new PostProcessor(SCR_WIDTH, SCR_HEIGHT);
    
    // Load and compile shaders - first try the glowing shaders, fall back to basic if they don't exist
    Shader* activeShader = nullptr;
    try {
        activeShader = new Shader("../shaders/glowing.vert", "../shaders/glowing.frag");
        std::cout << "Successfully loaded glowing shaders" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load glowing shaders, falling back to basic: " << e.what() << std::endl;
        activeShader = new Shader("../shaders/basic.vert", "../shaders/basic.frag");
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
        std::cerr << "Failed to load diamond textures, using fallback red color: " << e.what() << std::endl;
    }
    
    // Emerald ore
    OreProperties emerald;
    emerald.name = "Emerald";
    emerald.color = glm::vec3(0.0f, 0.8f, 0.2f); // Green
    emerald.glowStrength = 1.8f;
    
    try {
        emerald.diffuseMap = loadTexture("../textures/emerald/diffuse.png");
        emerald.emissiveMap = loadTexture("../textures/emerald/emissive.png");
        ores.push_back(emerald);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load emerald textures: " << e.what() << std::endl;
    }
    
    // Redstone ore
    OreProperties redstone;
    redstone.name = "Redstone";
    redstone.color = glm::vec3(0.9f, 0.1f, 0.1f); // Red
    redstone.glowStrength = 2.5f;
    
    try {
        redstone.diffuseMap = loadTexture("../textures/redstone/diffuse.png");
        redstone.emissiveMap = loadTexture("../textures/redstone/emissive.png");
        ores.push_back(redstone);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load redstone textures: " << e.what() << std::endl;
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
    std::cout << " - Left/Right arrows: Switch between ore types" << std::endl;
    std::cout << " - B key: Toggle bloom effect on/off" << std::endl;
    std::cout << " - +/- keys: Increase/decrease bloom intensity" << std::endl;
    std::cout << " - ,/. keys: Decrease/increase bloom threshold" << std::endl;
    
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Process input
        processInput(window, ambientLight, currentOreIndex, bloomEnabled, bloomIntensity, bloomThreshold);
        
        // Begin rendering to post-processing framebuffer if bloom is enabled
        if (bloomEnabled) {
            postProcessor->beginRender();
        }
        
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
            activeShader->setMat4("model", model);
        }
        
        GLint viewLoc = glGetUniformLocation(activeShader->ID, "view");
        if (viewLoc != -1) {
            activeShader->setMat4("view", view);
        }
        
        GLint projLoc = glGetUniformLocation(activeShader->ID, "projection");
        if (projLoc != -1) {
            activeShader->setMat4("projection", projection);
        }
        
        // Set ore-specific properties and glowing parameters if the shader supports them
        GLint viewPosLoc = glGetUniformLocation(activeShader->ID, "viewPos");
        if (viewPosLoc != -1) {
            activeShader->setVec3("viewPos", cameraPos);
        }
        
        GLint ambientLightLoc = glGetUniformLocation(activeShader->ID, "ambientLight");
        if (ambientLightLoc != -1) {
            activeShader->setFloat("ambientLight", ambientLight);
        }
        
        // Set bloom-related uniforms
        GLint bloomThresholdLoc = glGetUniformLocation(activeShader->ID, "bloomThreshold");
        if (bloomThresholdLoc != -1) {
            activeShader->setFloat("bloomThreshold", bloomThreshold);
        }
        
        // Make sure we have a valid ore to render
        int oreIndex = currentOreIndex % ores.size();
        OreProperties& currentOre = ores[oreIndex];
        
        // Set ore color and glow strength if the shader supports these uniforms
        GLint oreColorLoc = glGetUniformLocation(activeShader->ID, "oreColor");
        if (oreColorLoc != -1) {
            activeShader->setVec3("oreColor", currentOre.color);
        }
        
        GLint glowStrengthLoc = glGetUniformLocation(activeShader->ID, "glowStrength");
        if (glowStrengthLoc != -1) {
            activeShader->setFloat("glowStrength", currentOre.glowStrength);
        }
        
        // Bind textures if the shader supports them and we have valid textures
        GLint diffuseTexLoc = glGetUniformLocation(activeShader->ID, "diffuseTexture");
        GLint emissiveTexLoc = glGetUniformLocation(activeShader->ID, "emissiveTexture");
        
        if (diffuseTexLoc != -1 && currentOre.diffuseMap != 0) {
            activeShader->setInt("diffuseTexture", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, currentOre.diffuseMap);
        }
        
        if (emissiveTexLoc != -1 && currentOre.emissiveMap != 0) {
            activeShader->setInt("emissiveTexture", 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, currentOre.emissiveMap);
        }
        
        // Draw cube
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        // Apply bloom post-processing if enabled
        if (bloomEnabled) {
            postProcessor->endRender();
            postProcessor->applyBloom(bloomThreshold, bloomIntensity, bloomBlurPasses);
        }
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Display current settings
        std::cout << "\rOre: " << currentOre.name 
                  << " | Ambient Light: " << ambientLight 
                  << " | Bloom: " << (bloomEnabled ? "ON" : "OFF")
                  << " | Intensity: " << bloomIntensity
                  << " | Threshold: " << bloomThreshold << std::flush;
    }
    
    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    
    delete activeShader;
    delete postProcessor;  // Clean up post-processor
    
    glfwTerminate();
    return 0;
}

// Process keyboard input
void processInput(GLFWwindow* window, float &ambientLight, int &currentOreIndex, 
                 bool &bloomEnabled, float &bloomIntensity, float &bloomThreshold) {
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
    
    // Toggle bloom effect
    static bool bPressed = false;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        if (!bPressed) {
            bloomEnabled = !bloomEnabled;
            bPressed = true;
        }
    } else {
        bPressed = false;
    }
    
    // Adjust bloom intensity
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) // Equal/plus key
        bloomIntensity = std::min(bloomIntensity + 0.05f, 3.0f);
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
        bloomIntensity = std::max(bloomIntensity - 0.05f, 0.0f);
        
    // Adjust bloom threshold
    if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) // Period/greater key
        bloomThreshold = std::min(bloomThreshold + 0.01f, 1.0f);
    if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
        bloomThreshold = std::max(bloomThreshold - 0.01f, 0.0f);
}

// Window resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    
    // Resize post-processor framebuffers
    if (postProcessor) {
        postProcessor->resize(width, height);
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