#version 330 core

in vec3 fragNormal;
in vec3 fragPos;
uniform vec3 objectColor;

uniform vec3 sunDir;

out vec4 FragColor;

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
    
    // Slight height-based fog (distant = slightly lighter)
    float fogFactor = clamp((fragPos.y - 5.0) / 50.0, 0.0, 0.3);
    finalColor = mix(finalColor, vec3(0.8, 0.85, 0.9), fogFactor);
    
    FragColor = vec4(finalColor, 1.0);
}
