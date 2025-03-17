#version 120

varying vec2 texCoord;

void main() {
    // Simple pass-through vertex shader
    gl_Position = ftransform();
    texCoord = gl_MultiTexCoord0.st;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}