#version 120

// Minecraft attributes (add these to fix warnings)
attribute vec4 mc_Entity;
attribute vec4 mc_midTexCoord;
attribute vec4 at_tangent;

// Vanilla varyings
varying vec2 texCoords;
varying vec4 lmcoord;

// These are causing warnings if not used in fragment shader
// But if your fragment shader needs them, keep them
varying vec3 fragPos;
varying vec3 normal;

void main() {
    // Pass standard data
    gl_Position = ftransform();
    texCoords = gl_MultiTexCoord0.st;
    lmcoord = gl_TextureMatrix[1] * gl_MultiTexCoord1;
    
    // Compute these only if your fragment shader will use them
    fragPos = gl_Vertex.xyz;
    normal = gl_Normal;
}