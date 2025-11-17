#version 330 core

in vec3 fragNormal;
in vec3 fragPos;
uniform vec3 objectColor;

uniform vec3 sunDir;

out vec4 FragColor;

// New: model-space Y to determine trunk vs foliage
in float modelY;
uniform vec3 trunkColor;
uniform vec3 foliageColor;
uniform float foliageStart; // model-space Y where foliage begins
uniform float foliageBlend; // smooth blend range

void main()
{
    // Normalize normals and sun direction
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(sunDir);
    
    // Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = vec3(0.8, 0.8, 0.8) * diff * objectColor;
    
    // Ambient lighting for depth
    vec3 ambient = vec3(0.4, 0.45, 0.5) * objectColor;
    
    // Combine lighting
    vec3 finalColor = ambient + diffuse;

    // Blend between trunk and foliage colors using model-space Y
    float t = smoothstep(foliageStart, foliageStart + foliageBlend, modelY);
    vec3 mixedBase = mix(trunkColor, foliageColor, t);
    finalColor *= mixedBase;
    
    // Slight height-based fog (distant = slightly lighter)
    float fogFactor = clamp((fragPos.y - 5.0) / 50.0, 0.0, 0.3);
    finalColor = mix(finalColor, vec3(0.8, 0.85, 0.9), fogFactor);
    
    FragColor = vec4(finalColor, 1.0);
}
