#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include <GL/glew.h>
#include "shader.h"

class PostProcessor {
public:
    // Constructor and destructor
    PostProcessor(unsigned int width, unsigned int height);
    ~PostProcessor();
    
    // Resize framebuffers when window size changes
    void resize(unsigned int width, unsigned int height);
    
    // Start rendering to framebuffer
    void beginRender();
    
    // End rendering to framebuffer
    void endRender();
    
    // Apply bloom effect
    void applyBloom(float threshold, float intensity, int blur_passes);
    
    // Render a quad with the final result to the screen
    void renderToScreen();
    
    // Getter methods
    unsigned int getSceneTexture() const { return colorBuffers[0]; }
    unsigned int getBrightTexture() const { return colorBuffers[1]; }
    
private:
    // Screen dimensions
    unsigned int width, height;
    
    // Shader programs
    Shader *extractShader;
    Shader *blurShader;
    Shader *finalShader;
    
    // Framebuffers and textures
    unsigned int hdrFBO;
    unsigned int colorBuffers[2];
    unsigned int pingpongFBO[2];
    unsigned int pingpongBuffers[2];
    
    // Quad VAO for rendering post-process effects
    unsigned int quadVAO;
    
    // Initialize framebuffers
    void initFramebuffers();
    
    // Initialize quad geometry
    void initQuad();
    
    // Render quad to screen
    void renderQuad();
};

#endif