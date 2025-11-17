#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 WorldPos;

void main() {
    vec4 localPos = vec4(aPos.x, aPos.y, 0.0, 1.0);
    vec4 world = model * localPos;
    WorldPos = world.xyz;
    TexCoords = aTex;
    gl_Position = projection * view * world;
}
