#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aSnowDepth;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out float SnowDepth;
out float fogFactor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Nâng vertex lên theo độ sâu tuyết
    vec3 adjustedPos = aPos + aNormal * aSnowDepth;
    
    FragPos = vec3(model * vec4(adjustedPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    SnowDepth = aSnowDepth;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
    
    // Fog calculation
    float distance = length(FragPos - vec3(inverse(view)[3]));
    float fogDensity = 0.015;
    fogFactor = exp(-fogDensity * distance);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
}