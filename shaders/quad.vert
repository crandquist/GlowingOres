#version 410 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float bloomIntensity;

void main() {
    // Sample both the original scene and the blurred bright parts
    vec3 originalColor = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    
    // Add bloom to the original scene, scaled by bloom intensity
    originalColor += bloomColor * bloomIntensity;
    
    // Apply simple tone mapping to prevent oversaturation
    // This uses a basic Reinhard operator
    originalColor = originalColor / (originalColor + vec3(1.0));
    
    FragColor = vec4(originalColor, 1.0);
}