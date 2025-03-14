#version 410 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D image;
uniform bool horizontal;

// Gaussian weights for a 9-tap filter
const float weights[5] = float[5](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    // Get the size of a single texel (1 pixel in texture space)
    vec2 texOffset = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, TexCoords).rgb * weights[0]; // Central pixel weight
    
    // Apply blur in either horizontal or vertical direction
    if (horizontal) {
        // Horizontal blur pass
        for (int i = 1; i < 5; ++i) {
            // Add weighted samples from neighboring pixels
            result += texture(image, TexCoords + vec2(texOffset.x * i, 0.0)).rgb * weights[i];
            result += texture(image, TexCoords - vec2(texOffset.x * i, 0.0)).rgb * weights[i];
        }
    } else {
        // Vertical blur pass
        for (int i = 1; i < 5; ++i) {
            result += texture(image, TexCoords + vec2(0.0, texOffset.y * i)).rgb * weights[i];
            result += texture(image, TexCoords - vec2(0.0, texOffset.y * i)).rgb * weights[i];
        }
    }
    
    FragColor = vec4(result, 1.0);
}