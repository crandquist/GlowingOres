#version 120  // Minecraft prefers older GLSL versions for compatibility

// Minecraft built-in attributes
attribute vec4 mc_Entity;   // Contains block ID information
attribute vec4 mc_midTexCoord;
attribute vec4 at_tangent;

// Minecraft built-in uniforms
uniform mat4 gbufferModelView;
uniform mat4 gbufferModelViewInverse;
uniform mat4 gbufferProjection;
uniform vec3 shadowLightPosition;
uniform int worldTime;  // Can be used for time-based effects

// Outputs to fragment shader
varying vec3 fragPos;
varying vec3 normal;
varying vec2 texCoords;
varying vec4 lmcoord;       // Lightmap coordinates (Minecraft lighting)
varying float isOre;        // Flag to identify ore blocks

void main() {
    // Transform positions like in your glowing.vert, but using Minecraft matrices
    fragPos = (gbufferModelViewInverse * (gl_ModelViewMatrix * gl_Vertex)).xyz;
    normal = normalize(gl_NormalMatrix * gl_Normal);
    
    // Texture coordinates - Minecraft specific
    texCoords = gl_MultiTexCoord0.st;
    
    // Minecraft lightmap coordinates (crucial for lighting)
    lmcoord = gl_TextureMatrix[1] * gl_MultiTexCoord1;
    
    // Detect if this is an ore block (using Minecraft block IDs)
    // Diamond = 56, Emerald = 129, Redstone = 73, etc.
    isOre = 0.0;
    if (mc_Entity.x == 56.0 || mc_Entity.x == 129.0 || mc_Entity.x == 73.0) {
        isOre = 1.0;
    }
    
    // Final position calculation
    gl_Position = gl_ProjectionMatrix * (gl_ModelViewMatrix * gl_Vertex);
}