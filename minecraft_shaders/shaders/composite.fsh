#version 120

uniform sampler2D gcolor;
uniform float viewWidth;
uniform float viewHeight;
uniform float bloomThreshold = 0.5; // Configurable in shader.properties

void main() {
    vec2 texCoord = gl_TexCoord[0].st;
    vec3 color = texture2D(gcolor, texCoord).rgb;
    
    // Extract bright parts for bloom
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    
    if (brightness > bloomThreshold) {
        gl_FragData[0] = vec4(color, 1.0);
    } else {
        gl_FragData[0] = vec4(0.0, 0.0, 0.0, 1.0);
    }
}