#version 120

// Standard uniforms
uniform sampler2D texture;
uniform sampler2D lightmap;

// Varyings from vertex shader - must match!
varying vec2 texCoords;
varying vec4 lmcoord;
varying vec3 fragPos;  // Add these to match vertex output
varying vec3 normal;   // Add these to match vertex output

void main() {
    // Simple pass-through for testing
    vec4 color = texture2D(texture, texCoords);
    vec3 light = texture2D(lightmap, lmcoord.st).rgb;
    
    // Apply lighting
    color.rgb *= light;
    
    // Write to fragment output
    gl_FragData[0] = color;
}