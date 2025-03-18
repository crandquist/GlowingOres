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
#include <iomanip>
#include <sstream>
#include "shader.h"
#include "post_processor.h"  
#include "simple_text_renderer.h" // Using the simplified renderer

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, float &ambientLight, int &currentOreIndex, float &bloomIntensity, float &bloomThreshold);
unsigned int loadTexture(const char* path);
unsigned int createColorTexture(glm::vec3 color, int size = 16);

// Global variables
float ambientLight = 0.5f;      // Ambient light level (0.0 = dark, 1.0 = bright)
int currentOreIndex = 0;        // Current ore being displayed
float bloomIntensity = 1.0f;    // Bloom effect intensity
float bloomThreshold = 0.5f;    // Brightness threshold for bloom effect

// Track previous values to detect changes
static float prev_ambientLight = ambientLight;
static float prev_bloomIntensity = bloomIntensity;
static float prev_bloomThreshold = bloomThreshold;
static int prev_oreIndex = 0;

// Post-processor instance
PostProcessor* postProcessor = nullptr;
SimpleTextRenderer* textRenderer = nullptr; // Using our simple renderer instead

// State for value change indicators
struct ValueChangeIndicator {
    float timeLeft = 0.0f;
    bool increasing = false;
    bool decreasing = false;
};

ValueChangeIndicator ambientLightIndicator;
ValueChangeIndicator bloomIntensityIndicator;
ValueChangeIndicator bloomThresholdIndicator;
ValueChangeIndicator oreChangeIndicator;

// Struct to hold ore properties
struct OreProperties {
    std::string name;
    glm::vec3 color;
    float glowStrength;
    unsigned int diffuseMap;
    unsigned int emissiveMap;
};

// Implementation of framebuffer_size_callback - moved outside of main
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Update OpenGL viewport to match new window dimensions
    glViewport(0, 0, width, height);
    
    // Resize post-processor framebuffers if available
    if (postProcessor) {
        try {
            postProcessor->resize(width, height);
            std::cout << "Resized post-processor to " << width << "x" << height << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error resizing post-processor: " << e.what() << std::endl;
        }
    }
    
    // Update text renderer if it exists to match new window dimensions
    if (textRenderer) {
        try {
            // Delete old renderer and create a new one with updated dimensions
            delete textRenderer;
            textRenderer = new SimpleTextRenderer(width, height);
            std::cout << "Recreated text renderer for dimensions " << width << "x" << height << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error recreating text renderer: " << e.what() << std::endl;
            textRenderer = nullptr; // Set to null if recreation fails
        }
    }
}

// Implementation for processInput function
void processInput(GLFWwindow* window, float &ambientLight, int &currentOreIndex, float &bloomIntensity, float &bloomThreshold) {
    // Check for escape key to close the window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Adjust ambient light with up/down arrows
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        ambientLight += 0.01f;
        if (ambientLight > 1.0f) ambientLight = 1.0f;
        ambientLightIndicator.timeLeft = 1.0f;
        ambientLightIndicator.increasing = true;
        ambientLightIndicator.decreasing = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        ambientLight -= 0.01f;
        if (ambientLight < 0.0f) ambientLight = 0.0f;
        ambientLightIndicator.timeLeft = 1.0f;
        ambientLightIndicator.increasing = false;
        ambientLightIndicator.decreasing = true;
    }
    
    // Switch ore types with left/right arrows
    static bool rightKeyPressed = false;
    static bool leftKeyPressed = false;
    
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (!rightKeyPressed) {
            currentOreIndex++;
            oreChangeIndicator.timeLeft = 1.0f;
            oreChangeIndicator.increasing = true;
            oreChangeIndicator.decreasing = false;
            rightKeyPressed = true;
        }
    } else {
        rightKeyPressed = false;
    }
    
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (!leftKeyPressed) {
            currentOreIndex--;
            if (currentOreIndex < 0) currentOreIndex = 0;
            oreChangeIndicator.timeLeft = 1.0f;
            oreChangeIndicator.increasing = false;
            oreChangeIndicator.decreasing = true;
            leftKeyPressed = true;
        }
    } else {
        leftKeyPressed = false;
    }
    
    // Adjust bloom intensity with W/S keys
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        bloomIntensity += 0.05f;
        if (bloomIntensity > 5.0f) bloomIntensity = 5.0f;
        bloomIntensityIndicator.timeLeft = 1.0f;
        bloomIntensityIndicator.increasing = true;
        bloomIntensityIndicator.decreasing = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        bloomIntensity -= 0.05f;
        if (bloomIntensity < 0.0f) bloomIntensity = 0.0f;
        bloomIntensityIndicator.timeLeft = 1.0f;
        bloomIntensityIndicator.increasing = false;
        bloomIntensityIndicator.decreasing = true;
    }
    
    // Adjust bloom threshold with A/D keys
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        bloomThreshold += 0.01f;
        if (bloomThreshold > 1.0f) bloomThreshold = 1.0f;
        bloomThresholdIndicator.timeLeft = 1.0f;
        bloomThresholdIndicator.increasing = true;
        bloomThresholdIndicator.decreasing = false;
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        bloomThreshold -= 0.01f;
        if (bloomThreshold < 0.0f) bloomThreshold = 0.0f;
        bloomThresholdIndicator.timeLeft = 1.0f;
        bloomThresholdIndicator.increasing = false;
        bloomThresholdIndicator.decreasing = true;
    }
}

// Implementation for loadTexture function
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
            std::cerr << "Texture has unknown format: " << path << std::endl;
            format = GL_RGB; // Default fallback
        }
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);        
        stbi_image_free(data);
        
        std::cout << "Texture loaded successfully: " << path << ", dimensions: " << width << "x" << height << std::endl;
    } else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        std::cerr << "STB error: " << stbi_failure_reason() << std::endl;
        stbi_image_free(data);
        throw std::runtime_error("Failed to load texture");
    }
    
    return textureID;
}

// Implementation for createColorTexture function
unsigned int createColorTexture(glm::vec3 color, int size) {
    // Generate a simple colored texture for fallback when image textures are not available
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // Generate a solid color texture
    unsigned char* data = new unsigned char[size * size * 3];
    for (int i = 0; i < size * size * 3; i += 3) {
        // Convert color from 0-1 range to 0-255
        data[i] = static_cast<unsigned char>(color.r * 255.0f);
        data[i + 1] = static_cast<unsigned char>(color.g * 255.0f);
        data[i + 2] = static_cast<unsigned char>(color.b * 255.0f);
    }
    
    // Upload the texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Clean up
    delete[] data;
    
    std::cout << "Created color texture with RGB: (" 
              << color.r << ", " << color.g << ", " << color.b << ")" << std::endl;
    
    return textureID;
}

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
    
    // Enable alpha blending for text rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Print current directory to help with debugging file paths
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;
    
    // Initialize post-processor
    try {
        postProcessor = new PostProcessor(SCR_WIDTH, SCR_HEIGHT);
        std::cout << "Post-processor initialized successfully with bloom effect" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize post-processor: " << e.what() << std::endl;
        return -1;
    }
    
    // Initialize simple text renderer
    try {
        textRenderer = new SimpleTextRenderer(SCR_WIDTH, SCR_HEIGHT);
        std::cout << "Simple text renderer initialized successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize simple text renderer: " << e.what() << std::endl;
        // Continue without text rendering
    }
    
    // Load and compile shaders - first try the glowing shaders, fall back to basic if they don't exist
    Shader* activeShader = nullptr;
    try {
        activeShader = new Shader("shaders/glowing.vert", "shaders/glowing.frag");
        std::cout << "Successfully loaded glowing shaders" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load glowing shaders, falling back to basic: " << e.what() << std::endl;
        try {
            activeShader = new Shader("shaders/basic.vert", "shaders/basic.frag");
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
    
    // Define all overworld ore types
    std::vector<OreProperties> ores;
    
    // Diamond ore
    OreProperties diamond;
    diamond.name = "Diamond Ore";
    diamond.color = glm::vec3(0.0f, 0.8f, 1.0f); // Light blue
    diamond.glowStrength = 2.0f;
    
    // Emerald ore
    OreProperties emerald;
    emerald.name = "Emerald Ore";
    emerald.color = glm::vec3(0.0f, 1.0f, 0.0f); // Green
    emerald.glowStrength = 1.8f;
    
    // Redstone ore
    OreProperties redstone;
    redstone.name = "Redstone Ore";
    redstone.color = glm::vec3(1.0f, 0.0f, 0.0f); // Red
    redstone.glowStrength = 2.2f;
    
    // Gold ore
    OreProperties gold;
    gold.name = "Gold Ore";
    gold.color = glm::vec3(1.0f, 0.8f, 0.0f); // Golden yellow
    gold.glowStrength = 1.6f;
    
    // Iron ore
    OreProperties iron;
    iron.name = "Iron Ore";
    iron.color = glm::vec3(0.8f, 0.8f, 0.8f); // Silvery
    iron.glowStrength = 1.4f;
    
    // Lapis ore
    OreProperties lapis;
    lapis.name = "Lapis Ore";
    lapis.color = glm::vec3(0.0f, 0.0f, 0.8f); // Deep blue
    lapis.glowStrength = 1.7f;
    
    // Copper ore
    OreProperties copper;
    copper.name = "Copper Ore";
    copper.color = glm::vec3(0.8f, 0.4f, 0.1f); // Copper orange
    copper.glowStrength = 1.5f;
    
    // Try to load textures for each ore type
    try {
        // Diamond textures
        try {
            diamond.diffuseMap = loadTexture("textures/diamond/diffuse.png");
            diamond.emissiveMap = loadTexture("textures/diamond/emissive.png");
            std::cout << "Diamond textures loaded successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load diamond textures, using fallback colors: " << e.what() << std::endl;
            diamond.diffuseMap = createColorTexture(glm::vec3(0.2f, 0.4f, 0.8f));
            diamond.emissiveMap = createColorTexture(diamond.color);
        }
        ores.push_back(diamond);
        
        // Emerald textures
        try {
            emerald.diffuseMap = loadTexture("textures/emerald/diffuse.png");
            emerald.emissiveMap = loadTexture("textures/emerald/emissive.png");
            std::cout << "Emerald textures loaded successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load emerald textures, using fallback colors: " << e.what() << std::endl;
            emerald.diffuseMap = createColorTexture(glm::vec3(0.1f, 0.6f, 0.3f));
            emerald.emissiveMap = createColorTexture(emerald.color);
        }
        ores.push_back(emerald);
        
        // Redstone textures
        try {
            redstone.diffuseMap = loadTexture("textures/redstone/diffuse.png");
            redstone.emissiveMap = loadTexture("textures/redstone/emissive.png");
            std::cout << "Redstone textures loaded successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load redstone textures, using fallback colors: " << e.what() << std::endl;
            redstone.diffuseMap = createColorTexture(glm::vec3(0.6f, 0.1f, 0.1f));
            redstone.emissiveMap = createColorTexture(redstone.color);
        }
        ores.push_back(redstone);
        
        // Gold ore
        try {
            gold.diffuseMap = loadTexture("textures/gold/diffuse.png");
            gold.emissiveMap = loadTexture("textures/gold/emissive.png");
            std::cout << "Gold textures loaded successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load gold textures, using fallback colors: " << e.what() << std::endl;
            gold.diffuseMap = createColorTexture(glm::vec3(0.5f, 0.4f, 0.1f));
            gold.emissiveMap = createColorTexture(gold.color);
        }
        ores.push_back(gold);
        
        // Iron ore
        try {
            iron.diffuseMap = loadTexture("textures/iron/diffuse.png");
            diamond.emissiveMap = loadTexture("textures/iron/emissive.png");
            std::cout << "Iron textures loaded successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load iron textures, using fallback colors: " << e.what() << std::endl;
            iron.diffuseMap = createColorTexture(glm::vec3(0.5f, 0.5f, 0.5f));
            iron.emissiveMap = createColorTexture(iron.color);
        }
        ores.push_back(iron);
        
        
        // Lapis ore
        try {
            lapis.diffuseMap = loadTexture("textures/lapis/diffuse.png");
            lapis.emissiveMap = loadTexture("textures/lapis/emissive.png");
            std::cout << "Lapis textures loaded successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load lapis textures, using fallback colors: " << e.what() << std::endl;
            lapis.diffuseMap = createColorTexture(glm::vec3(0.1f, 0.1f, 0.5f));
            lapis.emissiveMap = createColorTexture(lapis.color);
        }
        ores.push_back(lapis);
        
        // Copper ore
        try {
            copper.diffuseMap = loadTexture("textures/copper/diffuse.png");
            lapis.emissiveMap = loadTexture("textures/copper/emissive.png");
            std::cout << "Copper textures loaded successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to load copper textures, using fallback colors: " << e.what() << std::endl;
            copper.diffuseMap = createColorTexture(glm::vec3(0.6f, 0.3f, 0.1f));
            copper.emissiveMap = createColorTexture(copper.color);
        }
        ores.push_back(copper);
        
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error setting up ore textures: " << e.what() << std::endl;
        
        // If we have a catastrophic failure, at least add one default ore
        if (ores.empty()) {
            diamond.diffuseMap = createColorTexture(glm::vec3(0.2f, 0.4f, 0.8f));
            diamond.emissiveMap = createColorTexture(diamond.color);
            ores.push_back(diamond);
        }
    }
    
    // Camera position
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    
    // Print instructions in the console
    std::cout << "Controls:" << std::endl;
    std::cout << " - Up/Down arrows: Adjust ambient light level" << std::endl;
    std::cout << " - Left/Right arrows: Switch between ore types" << std::endl;
    std::cout << " - W/S keys: Adjust bloom intensity" << std::endl;
    std::cout << " - A/D keys: Adjust bloom threshold" << std::endl;
    std::cout << " - ESC: Exit program" << std::endl;
    
    // Timing variables for animation
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Process input
        processInput(window, ambientLight, currentOreIndex, bloomIntensity, bloomThreshold);
        
        // Update value change indicators
        if (ambientLightIndicator.timeLeft > 0.0f)
            ambientLightIndicator.timeLeft -= deltaTime;
        if (bloomIntensityIndicator.timeLeft > 0.0f)
            bloomIntensityIndicator.timeLeft -= deltaTime;
        if (bloomThresholdIndicator.timeLeft > 0.0f)
            bloomThresholdIndicator.timeLeft -= deltaTime;
        if (oreChangeIndicator.timeLeft > 0.0f)
            oreChangeIndicator.timeLeft -= deltaTime;
        
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
        
        // Set bloom threshold for the shader (if it supports it)
        GLint bloomThresholdLoc = glGetUniformLocation(activeShader->ID, "bloomThreshold");
        if (bloomThresholdLoc != -1) {
            glUniform1f(bloomThresholdLoc, bloomThreshold);
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
        
        // End rendering to framebuffer
        postProcessor->endRender();
        
        // Apply bloom effect and render to screen
        postProcessor->applyBloom(bloomThreshold, bloomIntensity, 10);
        
        // Render text indicators on screen if we have a text renderer
        if (textRenderer) {
            // Create a vector of name-value pairs for the controls
            std::vector<std::pair<std::string, float>> values = {
                {"Ore Type", static_cast<float>(oreIndex)},
                {"Ambient Light", ambientLight},
                {"Bloom Intensity", bloomIntensity},
                {"Bloom Threshold", bloomThreshold}
            };
            
            // Render value bars
            textRenderer->renderValueDisplays(values, 20.0f, SCR_HEIGHT - 100.0f, 30.0f);
            
            // Render direction indicators for changing values
            float indicatorX = 230.0f;
            
            // Ambient light change indicator
            if (ambientLightIndicator.timeLeft > 0.0f) {
                textRenderer->renderDirectionIndicator(
                    indicatorX, SCR_HEIGHT - 130.0f, 
                    ambientLightIndicator.increasing, 
                    true, 
                    glm::vec4(0.2f, 0.6f, 1.0f, 1.0f)
                );
            }
            
            // Bloom intensity change indicator
            if (bloomIntensityIndicator.timeLeft > 0.0f) {
                textRenderer->renderDirectionIndicator(
                    indicatorX, SCR_HEIGHT - 160.0f, 
                    bloomIntensityIndicator.increasing, 
                    true, 
                    glm::vec4(1.0f, 0.6f, 0.2f, 1.0f)
                );
            }
            
            // Bloom threshold change indicator
            if (bloomThresholdIndicator.timeLeft > 0.0f) {
                textRenderer->renderDirectionIndicator(
                    indicatorX, SCR_HEIGHT - 190.0f, 
                    bloomThresholdIndicator.increasing, 
                    true, 
                    glm::vec4(0.2f, 1.0f, 0.6f, 1.0f)
                );
            }
            
            // Render control help indicators
            textRenderer->renderQuad(20.0f, 50.0f, 200.0f, 120.0f, glm::vec4(0.1f, 0.1f, 0.1f, 0.7f));
            
            // Render labeled bars for controls
            textRenderer->renderValueIndicator(30.0f, 140.0f, 180.0f, 20.0f, ambientLight, 0.0f, 1.0f, 
                                              glm::vec4(0.2f, 0.6f, 1.0f, 1.0f));
            textRenderer->renderValueIndicator(30.0f, 110.0f, 180.0f, 20.0f, bloomIntensity, 0.0f, 5.0f, 
                                              glm::vec4(1.0f, 0.6f, 0.2f, 1.0f));
            textRenderer->renderValueIndicator(30.0f, 80.0f, 180.0f, 20.0f, bloomThreshold, 0.0f, 1.0f, 
                                              glm::vec4(0.2f, 1.0f, 0.6f, 1.0f));
        }
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        // Only print when values change
        if (prev_ambientLight != ambientLight || prev_bloomIntensity != bloomIntensity || 
            prev_bloomThreshold != bloomThreshold || prev_oreIndex != oreIndex) {
            
            // Clear the entire line before printing new content
            std::cout << "\r\033[K";  // \r returns to start, \033[K clears to end of line
            std::cout << "Ore: " << std::setw(12) << std::left << currentOre.name
                    << " | Ambient Light: " << std::fixed << std::setprecision(2) << ambientLight 
                    << " | Bloom Intensity: " << bloomIntensity 
                    << " | Bloom Threshold: " << bloomThreshold 
                    << std::endl;  // Use endl instead of flush
            
            // Update previous values
            prev_ambientLight = ambientLight;
            prev_bloomIntensity = bloomIntensity;
            prev_bloomThreshold = bloomThreshold;
            prev_oreIndex = oreIndex;
        }
    }

    // Clean up
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    
    delete activeShader;
    delete postProcessor;
    if (textRenderer) delete textRenderer;
    
    glfwTerminate();
    return 0;
}