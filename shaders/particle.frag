#version 330 core
in vec2 TexCoord;
in vec3 FragPos;
in float fogFactor;

out vec4 FragColor;

uniform vec4 color;
uniform float rotation;

void main() {
    // Create snowflake pattern
    vec2 coord = TexCoord * 2.0 - 1.0;
    
    // Rotate coordinates
    float s = sin(rotation);
    float c = cos(rotation);
    mat2 rotMat = mat2(c, -s, s, c);
    coord = rotMat * coord;
    
    float dist = length(coord);
    
    // Snowflake shape với 6 cánh
    float angle = atan(coord.y, coord.x);
    float numArms = 6.0;
    float armPattern = abs(sin(angle * numArms));
    
    // Tạo hình dạng bông tuyết
    float snowflake = smoothstep(0.9, 0.3, dist) * (0.7 + 0.3 * armPattern);
    snowflake *= smoothstep(1.0, 0.8, dist);
    
    // Add soft glow
    float glow = exp(-dist * 3.0) * 0.3;
    snowflake += glow;
    
    // Apply color và alpha
    vec4 snowColor = vec4(color.rgb, color.a * snowflake);
    
    // Mix with fog color
    vec3 fogColor = vec3(0.6, 0.65, 0.7);
    snowColor.rgb = mix(fogColor, snowColor.rgb, fogFactor);
    
    // Discard transparent fragments
    if (snowColor.a < 0.01)
        discard;
    
    FragColor = snowColor;
}