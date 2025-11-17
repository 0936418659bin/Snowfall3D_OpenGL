#version 330 core
layout(location = 0) in vec2 aPos; // xy in quad
layout(location = 1) in vec2 aUV;

layout(location = 4) in vec4 instanceMat0;
layout(location = 5) in vec4 instanceMat1;
layout(location = 6) in vec4 instanceMat2;
layout(location = 7) in vec4 instanceMat3;
layout(location = 8) in float instanceSeed;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 camRight;
uniform vec3 camUp;
uniform float time;
uniform vec3 windDir;

out vec2 TexCoord;
out float vSeed;

mat4 getInstanceMat() { return mat4(instanceMat0, instanceMat1, instanceMat2, instanceMat3); }

vec3 ApplyWind(vec3 pos, float seed, float t) {
    float h = clamp(pos.y / 2.5, 0.0, 1.0);
    float b = t * 0.8 + seed * 10.0;
    float sx = sin(b + pos.x * 0.35) * 0.08;
    float sz = cos(b * 1.1 + pos.z * 0.25) * 0.06;
    return vec3(sx, 0.0, sz) * (1.0 - h);
}

void main() {
    mat4 model = getInstanceMat();
    // center pos in object space
    vec4 worldCenter = model * vec4(0.0, 0.2, 0.0, 1.0);  // offset up tree slightly
    // compute oriented quad offset
    float scale = 0.5; // leaf card scale
    vec3 right = normalize(camRight);
    vec3 up = normalize(camUp);
    // multiple leaves per tree for fuller appearance
    float leafIdx = mod(float(gl_InstanceID) * 3.7, 8.0);
    vec3 offset = right * aPos.x * scale * (0.8 + 0.4 * sin(leafIdx)) 
                + up * aPos.y * scale * (0.8 + 0.3 * cos(leafIdx * 1.3));
    // wind with per-leaf variation
    vec3 windOff = ApplyWind(vec3(offset), instanceSeed + leafIdx * 0.1, time);
    vec4 worldPos = vec4(worldCenter.xyz + offset + windOff, 1.0);
    TexCoord = aUV;
    vSeed = instanceSeed;
    gl_Position = projection * view * worldPos;
}
