#version 410 core

// Two output colors: one for the rendered scene and one for bright parts
layout (location = 0) out vec4 FragColor;      // Main color output
layout (location = 1) out vec4 BrightColor;    // Bright parts for bloom

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Textures
uniform sampler2D diffuseTexture;   // Base texture (ore texture)
uniform sampler2D emissiveTexture;   // Emissive mask (where the ore glows)

// Lighting parameters
uniform vec3 viewPos;               // Camera position
uniform float ambientLight;         // Ambient light level (0.0 to 1.0)
uniform vec3 oreColor;              // Color of the ore's glow
uniform float glowStrength;         // Base strength of the glow

// Bloom threshold - any pixels brighter than this go into the bright buffer
uniform float bloomThreshold;

void main() {
    // Sample textures
    vec4 diffuseColor = texture(diffuseTexture, TexCoords);
    vec4 emissiveMask = texture(emissiveTexture, TexCoords);
    
    // Calculate basic lighting
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Ambient lighting
    vec3 ambient = ambientLight * diffuseColor.rgb;
    
    // Calculate emissive component (the glow)
    float emissiveStrength = emissiveMask.r * glowStrength;
    
    // Dynamically adjust glow based on ambient light
    // Glow becomes more intense as ambient light decreases
    float dynamicGlow = emissiveStrength * (1.0 - ambientLight);
    
    // Combine ambient lighting with glow
    vec3 result = ambient + (oreColor * dynamicGlow);
    
    // Output the final color
    FragColor = vec4(result, diffuseColor.a);
    
    // Check if the pixel is bright enough for bloom
    // We'll use the emissive parts only
    float brightness = dot(oreColor * dynamicGlow, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > bloomThreshold) {
        BrightColor = vec4(oreColor * dynamicGlow, 1.0);
    } else {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}