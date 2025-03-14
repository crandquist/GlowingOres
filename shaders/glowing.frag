#version 410 core

out vec4 FragColor;

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
    
    FragColor = vec4(result, diffuseColor.a);
}