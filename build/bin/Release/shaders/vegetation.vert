#version 330 core

// Per-vertex attributes
layout(location = 0) in vec3 position;

// Per-instance attributes (instance matrix as 4 vec4s)
layout(location = 4) in vec4 instanceMat0;
layout(location = 5) in vec4 instanceMat1;
layout(location = 6) in vec4 instanceMat2;
layout(location = 7) in vec4 instanceMat3;
layout(location = 8) in float instanceSeed;

// Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform float time;
uniform vec3 windDir;

out vec3 fragNormal;
out vec3 fragPos;
out float modelY;

// Wind sway DISABLED - trees stand still
// To re-enable wind, uncomment the function below and replace ApplyWind call
/*
vec3 ApplyWind(vec3 pos, float seed, float windTime) {
    float heightFactor = clamp(pos.y / 2.5, 0.0, 1.0);
    float base = windTime * 0.8 + seed * 10.0;
    float swayX = sin(base + pos.x * 0.35) * 0.12;
    float swayZ = cos(base * 1.1 + pos.z * 0.25) * 0.09;
    return vec3(swayX, 0.0, swayZ) * (1.0 - heightFactor);
}
*/

void main()
{
    // Reconstruct instance model matrix
    mat4 model = mat4(instanceMat0, instanceMat1, instanceMat2, instanceMat3);

    // No wind sway - trees stay still
    vec3 swayedPos = position;

    // Transform to world
    vec4 worldPos = model * vec4(swayedPos, 1.0);
    fragPos = vec3(worldPos);
    // Pass model-space Y for color blending (foliage vs trunk)
    modelY = position.y;

    // Approximate normal
    fragNormal = normalize(mat3(model) * normalize(position));

    gl_Position = projection * view * worldPos;
}
