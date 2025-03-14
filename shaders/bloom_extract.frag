#version 410 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform float threshold;

void main() {
    // Sample the scene texture
    vec3 color = texture(scene, TexCoords).rgb;
    
    // Calculate brightness of the pixel (using luminance formula)
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    // If the pixel is bright enough, keep its color; otherwise set it to black
    if (brightness > threshold) {
        FragColor = vec4(color, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}