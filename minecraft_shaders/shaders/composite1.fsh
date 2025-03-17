#version 120

uniform sampler2D gcolor;
uniform float viewWidth;
uniform float viewHeight;
uniform bool horizontal = true; // Will be true for composite1, false for composite2

void main() {
    vec2 texCoord = gl_TexCoord[0].st;
    vec2 texelSize = 1.0 / vec2(viewWidth, viewHeight);
    
    // Gaussian weights (same as in your blur.frag)
    float weights[5] = float[5](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    
    vec3 result = texture2D(gcolor, texCoord).rgb * weights[0];
    
    if (horizontal) {
        // Horizontal blur
        for (int i = 1; i < 5; ++i) {
            result += texture2D(gcolor, texCoord + vec2(texelSize.x * i, 0.0)).rgb * weights[i];
            result += texture2D(gcolor, texCoord - vec2(texelSize.x * i, 0.0)).rgb * weights[i];
        }
    } else {
        // Vertical blur
        for (int i = 1; i < 5; ++i) {
            result += texture2D(gcolor, texCoord + vec2(0.0, texelSize.y * i)).rgb * weights[i];
            result += texture2D(gcolor, texCoord - vec2(0.0, texelSize.y * i)).rgb * weights[i];
        }
    }
    
    gl_FragData[0] = vec4(result, 1.0);
}