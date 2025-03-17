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
varying float blockId;      // Block ID to identify specific ore types

void main() {
    // Transform positions using Minecraft matrices
    fragPos = (gbufferModelViewInverse * (gl_ModelViewMatrix * gl_Vertex)).xyz;
    normal = normalize(gl_NormalMatrix * gl_Normal);
    
    // Texture coordinates - Minecraft specific
    texCoords = gl_MultiTexCoord0.st;
    
    // Minecraft lightmap coordinates (crucial for lighting)
    lmcoord = gl_TextureMatrix[1] * gl_MultiTexCoord1;
    
    // Pass the block ID to the fragment shader
    blockId = mc_Entity.x;
    
    // Detect if this is an ore block (using Minecraft block IDs)
    isOre = 0.0;
    
    // Check for all traditional ore block IDs
    if (mc_Entity.x == 16.0 ||   // Coal Ore
        mc_Entity.x == 15.0 ||   // Iron Ore
        mc_Entity.x == 14.0 ||   // Gold Ore
        mc_Entity.x == 56.0 ||   // Diamond Ore
        mc_Entity.x == 21.0 ||   // Lapis Ore
        mc_Entity.x == 73.0 ||   // Redstone Ore
        mc_Entity.x == 74.0 ||   // Lit Redstone Ore
        mc_Entity.x == 129.0 ||  // Emerald Ore
        mc_Entity.x == 153.0) {  // Nether Quartz Ore
        isOre = 1.0;
    }
    
    // For modern blocks (Deepslate variants, Copper, Ancient Debris) 
    // OptiFine typically assigns custom numeric IDs in the range of 10000+
    // You can use these values if defined in your OptiFine configuration
    
    // Final position calculation
    gl_Position = gl_ProjectionMatrix * (gl_ModelViewMatrix * gl_Vertex);
}