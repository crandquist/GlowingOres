#version 120

// Minecraft textures
uniform sampler2D texture;  // Base texture (albedo)
uniform sampler2D lightmap; // Minecraft's lightmap (torch light + sky light)

// Custom uniforms (can be set in shader.properties)
uniform float ambientLight = 0.5;  // Default value
uniform float glowStrength = 2.0;  // Default value

// From vertex shader
varying vec3 fragPos;
varying vec3 normal;
varying vec2 texCoords;
varying vec4 lmcoord;
varying float isOre;

// Define ore colors - you can add more or parameterize these
const vec3 diamondColor = vec3(0.0, 0.8, 1.0);          // Light blue
const vec3 emeraldColor = vec3(0.0, 1.0, 0.0);          // Green
const vec3 redstoneColor = vec3(1.0, 0.0, 0.0);         // Red
const vec3 goldColor = vec3(1.0, 0.8, 0.0);             // Golden yellow
const vec3 coalColor = vec3(0.2, 0.2, 0.3);             // Dark gray with slight blue
const vec3 ironColor = vec3(0.8, 0.8, 0.8);             // Silvery
const vec3 lapisColor = vec3(0.0, 0.0, 0.8);            // Deep blue
const vec3 copperColor = vec3(0.8, 0.4, 0.1);           // Copper orange
const vec3 netherGoldColor = vec3(1.0, 0.6, 0.0);       // Brighter orange-gold
const vec3 quartzColor = vec3(0.9, 0.9, 0.9);           // White with slight glow
const vec3 ancientDebrisColor = vec3(0.5, 0.3, 0.6);    // Purple-brown

void main() {
    // Sample the base texture
    vec4 baseColor = texture2D(texture, texCoords);
    
    // Sample Minecraft's lightmap
    vec3 lightLevel = texture2D(lightmap, lmcoord.st).rgb;
    
    // Default output is just the textured block
    vec4 finalColor = baseColor;
    
    // Only apply glow effects to ore blocks
    if (isOre > 0.5) {
        vec3 oreColor = diamondColor; // Default fallback color
        
        // Method 1: If you have the block ID available through mc_Entity from the vertex shader
        int blockId = int(mc_Entity.x);
        
        // Determine which ore based on block ID
        if (blockId == 16) oreColor = coalColor;           // Coal Ore
        else if (blockId == 15) oreColor = ironColor;      // Iron Ore
        else if (blockId == 14) oreColor = goldColor;      // Gold Ore
        else if (blockId == 56) oreColor = diamondColor;   // Diamond Ore
        else if (blockId == 21) oreColor = lapisColor;     // Lapis Ore
        else if (blockId == 73) oreColor = redstoneColor;  // Redstone Ore
        else if (blockId == 129) oreColor = emeraldColor;  // Emerald Ore
        // Add more ore type checks as needed
        
        // Method 2 (Alternative): If block ID isn't available, you can try analyzing the texture
        // This is less reliable but can work in some cases
        /*
        vec3 avgColor = baseColor.rgb;
        float r = avgColor.r;
        float g = avgColor.g;
        float b = avgColor.b;
        
        // Very approximate texture detection based on color
        if (b > r && b > g) oreColor = diamondColor;       // Diamond or Lapis, blue-dominant
        else if (g > r && g > b) oreColor = emeraldColor;  // Emerald, green-dominant
        else if (r > g && r > b) oreColor = redstoneColor; // Redstone, red-dominant
        // And so on - this method is much less reliable
        */
        
        // Calculate dynamic glow based on light level
        float brightness = dot(lightLevel, vec3(0.2126, 0.7152, 0.0722));
        float dynamicGlow = glowStrength * (1.0 - brightness);
        
        // Add the glow to the base color
        finalColor.rgb += oreColor * dynamicGlow;
    }
    
    // Output to gbuffer
    gl_FragData[0] = finalColor;
}