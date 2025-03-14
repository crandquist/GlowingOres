#include "post_processor.h"
#include <iostream>

PostProcessor::PostProcessor(unsigned int width, unsigned int height) 
    : width(width), height(height) {
    
    // Load shaders
    extractShader = new Shader("../shaders/quad.vert", "../shaders/bloom_extract.frag");
    blurShader = new Shader("../shaders/quad.vert", "../shaders/blur.frag");
    finalShader = new Shader("../shaders/quad.vert", "../shaders/bloom_final.frag");
    
    // Initialize framebuffers and quad
    initFramebuffers();
    initQuad();
}

PostProcessor::~PostProcessor() {
    // Clean up resources
    delete extractShader;
    delete blurShader;
    delete finalShader;
    
    glDeleteFramebuffers(1, &hdrFBO);
    glDeleteTextures(2, colorBuffers);
    glDeleteFramebuffers(2, pingpongFBO);
    glDeleteTextures(2, pingpongBuffers);
    glDeleteVertexArrays(1, &quadVAO);
}

void PostProcessor::resize(unsigned int newWidth, unsigned int newHeight) {
    // If the screen dimensions change, we need to recreate our framebuffers
    width = newWidth;
    height = newHeight;
    
    // Clean up and reinitialize
    glDeleteFramebuffers(1, &hdrFBO);
    glDeleteTextures(2, colorBuffers);
    glDeleteFramebuffers(2, pingpongFBO);
    glDeleteTextures(2, pingpongBuffers);
    
    initFramebuffers();
}

void PostProcessor::beginRender() {
    // Bind the HDR framebuffer for scene rendering
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessor::endRender() {
    // Unbind the framebuffer, returning to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::applyBloom(float threshold, float intensity, int blur_passes) {
    // 1. Extract bright parts of the scene
    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[0]);
    glClear(GL_COLOR_BUFFER_BIT);
    
    extractShader->use();
    extractShader->setInt("scene", 0);
    extractShader->setFloat("threshold", threshold);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[1]); // Use the bright buffer from the HDR FBO
    
    renderQuad();
    
    // 2. Apply gaussian blur (ping-pong between two framebuffers)
    bool horizontal = true;
    
    blurShader->use();
    blurShader->setInt("image", 0);
    
    // Multiple blur passes for smoother results
    for (int i = 0; i < blur_passes; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal ? 1 : 0]);
        blurShader->setBool("horizontal", horizontal);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffers[horizontal ? 0 : 1]);
        
        renderQuad();
        
        horizontal = !horizontal;
    }
    
    // 3. Combine the original scene with the blurred bright parts (render to screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    finalShader->use();
    finalShader->setInt("scene", 0);
    finalShader->setInt("bloomBlur", 1);
    finalShader->setFloat("bloomIntensity", intensity);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongBuffers[!horizontal ? 1 : 0]);
    
    renderQuad();
}

void PostProcessor::renderToScreen() {
    // Render the scene texture directly to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    finalShader->use();
    finalShader->setInt("scene", 0);
    finalShader->setInt("bloomBlur", 0);
    finalShader->setFloat("bloomIntensity", 0.0f); // No bloom
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
    
    renderQuad();
}

void PostProcessor::initFramebuffers() {
    // 1. Create HDR framebuffer with two color attachments
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    
    // Create two color buffers: one for the rendered scene and one for the bright parts
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // Attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    
    // Create and attach a renderbuffer object for depth testing
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    
    // Tell OpenGL which color attachments we'll use for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    
    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
    }
    
    // 2. Create ping-pong framebuffers for blurring
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffers);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffers[i], 0);
        
        // Check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Ping-pong framebuffer " << i << " not complete!" << std::endl;
        }
    }
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::initQuad() {
    // Create a VAO with a single quad (two triangles) to render our effects
    float quadVertices[] = {
        // positions        // texture coordinates
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };
    
    unsigned int quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void PostProcessor::renderQuad() {
    // Render the quad with current bound framebuffer and shader
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}