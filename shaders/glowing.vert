#version 410 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Calculate fragment position in world space (for lighting)
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Transform normals to world space
    // The normal matrix is the transpose of the inverse of the model matrix
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Pass texture coordinates to fragment shader
    TexCoords = aTexCoords;
    
    // Calculate final position
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}