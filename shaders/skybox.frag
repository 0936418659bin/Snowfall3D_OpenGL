#version 330 core
in vec3 TexCoords;
in vec3 WorldPos;

out vec4 FragColor;

uniform float timeOfDay; // 0..24
uniform vec3 sunDirection;

// base colors
const vec3 dayTop = vec3(0.52, 0.7, 0.95);
const vec3 dayHorizon = vec3(0.95, 0.8, 0.6);
const vec3 nightTop = vec3(0.02, 0.03, 0.08);
const vec3 nightHorizon = vec3(0.08, 0.05, 0.15);

float hash21(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// simple fbm using sines for subtle banding-free noise
float fbm(vec2 p) {
    float f = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 5; ++i) {
        f += amp * sin(p.x * (float(i)+1.3) * 0.8 + p.y * (float(i)+0.7) * 0.6 + float(i)*1.7);
        p *= 1.9;
        amp *= 0.5;
    }
    return f * 0.5 + 0.5;
}

void main() {
    vec3 dir = normalize(WorldPos);
    float elevation = clamp(dir.y, -1.0, 1.0);

    // sun elevation factor 0..1
    float sunElev = clamp((sunDirection.y + 0.1) / 1.1, 0.0, 1.0);

    // blend day/night by sun elevation
    vec3 topColor = mix(nightTop, dayTop, sunElev);
    vec3 horizonColor = mix(nightHorizon, dayHorizon, sunElev);

    float t = smoothstep(-0.2, 0.9, elevation);
    vec3 sky = mix(horizonColor, topColor, t);

    // sun disk glow
    float sunFactor = max(0.0, dot(dir, normalize(sunDirection)));
    float sunDisk = pow(sunFactor, 200.0) * (0.9 + 0.6 * sunElev);
    sky += vec3(1.0, 0.95, 0.8) * sunDisk * 0.8;

    // subtle high-altitude haze
    float haze = exp(-max(0.0, dir.y) * 4.0) * 0.12;
    sky += vec3(0.9, 0.95, 1.0) * haze * sunElev;

    // procedural distant cloud layer tinting
    float cloudLayer = fbm(WorldPos.xz * 0.02 + vec2(timeOfDay * 0.02));
    float cloudMask = smoothstep(0.55, 0.7, cloudLayer);
    vec3 cloudColor = vec3(1.0) * (0.6 + 0.4 * sunElev);
    sky = mix(sky, mix(sky, cloudColor, 0.6), cloudMask * 0.45);

    FragColor = vec4(sky, 1.0);
}