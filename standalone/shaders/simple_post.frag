// shaders/simple_post.frag
#version 410 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    FragColor = texture(screenTexture, TexCoords);
    // Optional: simple effect like inverting colors
    // FragColor = vec4(1.0 - texture(screenTexture, TexCoords).rgb, 1.0);
}