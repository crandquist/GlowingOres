#version 120

// Minecraft textures
uniform sampler2D texture;  // Base texture (albedo)
uniform sampler2D lightmap; // Minecraft's lightmap (torch light + sky light)

// Custom uniforms (can be set in shader.properties)
uniform float ambientLight = 0.5;  // Default value
uniform float glowStrength = 2.0;  // Default value

// From vertex shader
varying vec3 fragPos;
varying vec3 normal;
varying vec2 texCoords;
varying vec4 lmcoord;
varying float isOre;

// Define ore colors - you can add more or parameterize these
const vec3 diamondColor = vec3(0.0, 0.8, 1.0);  // Light blue
const vec3 emeraldColor = vec3(0.0, 1.0, 0.0);  // Green
const vec3 redstoneColor = vec3(1.0, 0.0, 0.0); // Red

void main() {
    // Sample the base texture
    vec4 baseColor = texture2D(texture, texCoords);
    
    // Sample Minecraft's lightmap
    vec3 lightLevel = texture2D(lightmap, lmcoord.st).rgb;
    
    // Default output is just the textured block
    vec4 finalColor = baseColor;
    
    // Only apply glow effects to ore blocks
    if (isOre > 0.5) {
        // We'd need to determine which ore type this is based on texture or other properties
        // For now, let's assume it's diamond (you can extend this logic)
        vec3 oreColor = diamondColor;
        
        // Calculate dynamic glow based on light level
        // Ores should glow more in darkness (caves) and less in bright areas
        float brightness = dot(lightLevel, vec3(0.2126, 0.7152, 0.0722));
        float dynamicGlow = glowStrength * (1.0 - brightness);
        
        // Add the glow to the base color
        finalColor.rgb += oreColor * dynamicGlow;
    }
    
    // Output to gbuffer
    gl_FragData[0] = finalColor;
}