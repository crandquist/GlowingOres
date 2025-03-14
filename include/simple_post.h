// Simple post-processor implementation (include/simple_post.h)
#ifndef SIMPLE_POST_H
#define SIMPLE_POST_H

#include <GL/glew.h>
#include "shader.h"

class SimplePostProcessor {
public:
    SimplePostProcessor(unsigned int width, unsigned int height);
    ~SimplePostProcessor();
    
    void beginRender();
    void endRender();
    void renderToScreen();
    
private:
    unsigned int width, height;
    unsigned int framebuffer;
    unsigned int textureColorBuffer;
    unsigned int quadVAO;
    Shader* screenShader;
    
    void initFramebuffer();
    void initQuad();
};

#endif