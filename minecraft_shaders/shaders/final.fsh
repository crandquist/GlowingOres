#version 120

uniform sampler2D gcolor;      // Original scene
uniform sampler2D composite;    // Blurred bright areas
uniform float bloomIntensity = 1.0;

void main() {
    vec2 texCoord = gl_TexCoord[0].st;
    
    // Get the main scene color and bloom
    vec3 sceneColor = texture2D(gcolor, texCoord).rgb;
    vec3 bloomColor = texture2D(composite, texCoord).rgb;
    
    // Add bloom with configurable intensity
    vec3 finalColor = sceneColor + bloomColor * bloomIntensity;
    
    // Optional: tone mapping for HDR effect
    finalColor = finalColor / (finalColor + vec3(1.0));
    
    gl_FragColor = vec4(finalColor, 1.0);
}