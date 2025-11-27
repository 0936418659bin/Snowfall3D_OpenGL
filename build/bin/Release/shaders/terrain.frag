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
in vec3 Tangent;
in vec3 Bitangent;

out vec4 FragColor;

uniform vec3 viewPos;
uniform DirectionalLight dirLight;
uniform int numPointLights;
uniform PointLight pointLights[4];

// Procedural Perlin-like noise
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), f.x),
               mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), f.x), f.y);
}

// Generate normal from noise for surface detail
vec3 GenerateDetailNormal(vec2 texCoord, vec3 normal) {
    float delta = 0.01;
    float h0 = noise(texCoord);
    float hx = noise(texCoord + vec2(delta, 0.0));
    float hy = noise(texCoord + vec2(0.0, delta));
    
    vec3 dx = vec3(delta, hx - h0, 0.0);
    vec3 dy = vec3(0.0, hy - h0, delta);
    vec3 detailNorm = normalize(cross(dy, dx));
    
    return normalize(normal + detailNorm * 0.3);
}

// PBR-inspired directional light
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 baseColor, float roughness, float metallic) {
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular with Fresnel
    float spec = pow(max(dot(normal, halfwayDir), 0.0), mix(32.0, 256.0, 1.0 - roughness));
    float fresnel = mix(0.04, 1.0, pow(1.0 - max(dot(halfwayDir, lightDir), 0.0), 5.0));
    
    // Energy conservation
    vec3 ks = mix(vec3(fresnel), baseColor, metallic);
    vec3 kd = baseColor * (1.0 - metallic);
    
    vec3 ambient = light.ambient * baseColor * 0.5;
    vec3 diffuse = light.diffuse * diff * kd;
    vec3 specular = light.specular * spec * ks;
    
    return ambient + diffuse + specular;
}

// PBR-inspired point light
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor, float roughness) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), mix(32.0, 128.0, roughness));
    
    // Attenuation with inverse square law
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    
    vec3 ambient = light.ambient * baseColor * 0.3;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = light.specular * spec;
    
    return (ambient + diffuse + specular) * attenuation;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Add detailed normal variation for surface smoothness
    norm = GenerateDetailNormal(TexCoord, norm);
    norm = normalize(norm);
    
    // Base colors with enhanced variety
    vec3 grassColor = vec3(0.18, 0.38, 0.18);
    vec3 snowColor = vec3(0.96, 0.98, 1.0);
    vec3 rockColor = vec3(0.42, 0.42, 0.46);
    
    // Enhanced slope calculation
    float slope = 1.0 - abs(dot(norm, vec3(0.0, 1.0, 0.0)));
    
    // Add terrain texture variation for realism
    float terrainNoise = noise(TexCoord * 0.5);
    vec3 baseColor = mix(grassColor, rockColor, smoothstep(0.25, 0.65, slope + terrainNoise * 0.1));
    
    // Apply snow layer with smoother transition
    float snowMix = smoothstep(-0.05, 0.35, SnowDepth);
    baseColor = mix(baseColor, snowColor, snowMix);
    
    // Material properties - snow is smoother/less rough
    float roughness = mix(0.6, 0.2, snowMix);
    float metallic = 0.0;
    
    // Add subtle color variation on snow (blue/purple tint in shadows)
    if (snowMix > 0.5) {
        baseColor += vec3(-0.02, -0.01, 0.02) * (snowMix - 0.5) * 2.0;
    }
    
    // Calculate lighting with PBR
    vec3 result = CalcDirLight(dirLight, norm, viewDir, baseColor, roughness, metallic);
    
    // Add point lights
    for(int i = 0; i < numPointLights && i < 4; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, baseColor, roughness);
    }
    
    // Apply fog with Rayleigh scattering effect
    vec3 fogColor = vec3(0.65, 0.7, 0.75);
    result = mix(fogColor, result, fogFactor);
    
    // Enhanced snow sparkle and subsurface scattering
    if (snowMix > 0.5) {
        // Specular highlights on snow
        float sparkle = pow(max(dot(viewDir, reflect(-dirLight.direction, norm)), 0.0), 256.0);
        result += sparkle * 0.4 * snowMix;
        
        // Subsurface scattering for snow depth
        float sss = pow(max(dot(norm, -dirLight.direction), 0.0), 1.5) * 0.2;
        result += vec3(sss * 0.15);
    }
    
    // Subtle ambient occlusion based on noise
    float ao = mix(1.0, 0.85, noise(TexCoord * 2.0) * 0.3);
    result *= ao;
    
    // Slight color grading for more cinematic look
    result = mix(result, vec3(dot(result, vec3(0.299, 0.587, 0.114))), 0.05);
    
    FragColor = vec4(result, 1.0);
}