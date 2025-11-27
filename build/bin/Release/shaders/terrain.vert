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
out vec3 Tangent;
out vec3 Bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Nâng vertex lên theo độ sâu tuyết
    vec3 adjustedPos = aPos + aNormal * aSnowDepth;
    
    FragPos = vec3(model * vec4(adjustedPos, 1.0));
    Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
    TexCoord = aTexCoord * 4.0;
    SnowDepth = aSnowDepth;
    
    // Compute tangent and bitangent for normal mapping
    vec3 Q1 = dFdx(FragPos);
    vec3 Q2 = dFdy(FragPos);
    vec2 st1 = dFdx(TexCoord);
    vec2 st2 = dFdy(TexCoord);
    
    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);
    T = normalize(T - dot(T, N) * N);
    Tangent = T;
    Bitangent = cross(N, T);
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
    
    // Fog calculation with better exponential falloff
    float distance = length(FragPos - vec3(inverse(view)[3]));
    float fogDensity = 0.012;
    fogFactor = exp(-fogDensity * distance * fogDensity * distance);
    fogFactor = clamp(fogFactor, 0.0, 1.0);
}