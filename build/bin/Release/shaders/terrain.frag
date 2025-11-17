#version 330 core
struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in float SnowDepth;
in float fogFactor;

out vec4 FragColor;

uniform vec3 viewPos;
uniform DirectionalLight dirLight;
uniform int numPointLights;
uniform PointLight pointLights[4];

// Calculate directional light
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 baseColor) {
    vec3 lightDir = normalize(-light.direction);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    
    // Combine
    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = light.specular * spec;
    
    return ambient + diffuse + specular;
}

// Calculate point light
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    
    // Combine
    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = light.specular * spec;
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return ambient + diffuse + specular;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Base colors
    vec3 grassColor = vec3(0.2, 0.4, 0.2);
    vec3 snowColor = vec3(0.95, 0.97, 1.0);
    vec3 rockColor = vec3(0.4, 0.4, 0.45);
    
    // Mix colors based on slope and snow depth
    float slope = 1.0 - abs(dot(norm, vec3(0.0, 1.0, 0.0)));
    vec3 baseColor = mix(grassColor, rockColor, smoothstep(0.3, 0.6, slope));
    
    // Apply snow layer
    float snowMix = smoothstep(0.0, 0.3, SnowDepth);
    baseColor = mix(baseColor, snowColor, snowMix);
    
    // Add procedural texture detail
    float noise = fract(sin(dot(TexCoord, vec2(12.9898, 78.233))) * 43758.5453);
    baseColor += vec3(noise * 0.05);
    
    // Calculate lighting
    vec3 result = CalcDirLight(dirLight, norm, viewDir, baseColor);
    
    // Add point lights
    for(int i = 0; i < numPointLights && i < 4; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, baseColor);
    }
    
    // Apply fog
    vec3 fogColor = vec3(0.6, 0.65, 0.7);
    result = mix(fogColor, result, fogFactor);
    
    // Add subtle sparkle to snow
    if (SnowDepth > 0.1) {
        float sparkle = pow(max(dot(viewDir, reflect(-dirLight.direction, norm)), 0.0), 128.0);
        result += sparkle * 0.3 * snowMix;
    }
    
    FragColor = vec4(result, 1.0);
}