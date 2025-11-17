#version 330 core
in vec2 TexCoord;
in float vSeed;

uniform vec3 objectColor;
uniform vec3 sunDir;

out vec4 FragColor;

void main() {
    // Procedural leaf shape (ellipse with variation)
    vec2 uv = TexCoord - vec2(0.5);
    float r = length(uv);
    // elongated ellipse
    float ellipse = length(vec2(uv.x * 0.7, uv.y)) / 0.5;
    float alpha = smoothstep(0.6, 0.45, ellipse);
    if (alpha < 0.1) discard;

    // Leaf color variation: lighter at edges, darker in center
    float edgeFade = pow(1.0 - r * 2.0, 1.5);
    vec3 leafBase = mix(vec3(0.12, 0.35, 0.12), vec3(0.2, 0.55, 0.18), edgeFade);

    // Seed-based variation (different leaves have different tints)
    float leafVar = fract(sin(vSeed * 43758.5453) * 43758.5453);
    leafBase = mix(leafBase, vec3(0.15, 0.45, 0.15), leafVar * 0.3);

    // Directional lighting for natural shading
    vec3 norm = normalize(vec3(uv.x * 0.3, 1.0, uv.y * 0.3));
    float diff = max(dot(norm, normalize(sunDir)), 0.0);
    vec3 lit = leafBase * (0.3 + 0.9 * diff);

    FragColor = vec4(lit, alpha);
}
