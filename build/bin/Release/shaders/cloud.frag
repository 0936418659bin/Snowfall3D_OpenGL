#version 330 core
in vec2 TexCoords;
in vec3 WorldPos;

out vec4 FragColor;

uniform float time;
uniform vec3 sunDir;
uniform float coverage;

// Simple hash and noise
float hash(float n) { return fract(sin(n) * 43758.5453123); }
float noise(vec2 x){
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0;
    float res = mix(mix(hash(n+0.0), hash(n+1.0), f.x), mix(hash(n+57.0), hash(n+58.0), f.x), f.y);
    return res;
}

float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < 6; ++i) {
        v += a * noise(p);
        p *= 2.0;
        a *= 0.5;
    }
    return v;
}

// Volumetric cloud density sampling
float SampleCloudDensity(vec2 samplePos, float height) {
    vec2 baseUV = samplePos * 0.02 + vec2(time * 0.01);
    float density = fbm(baseUV * 2.0);
    
    // Height-based falloff (clouds thinner at extremes)
    float heightMod = 1.0 - abs(height - 0.5) * 0.6;
    density *= heightMod;
    
    return density;
}

void main() {
    // Sample multiple slices for volumetric look (4 slices per tile)
    vec3 accumulatedColor = vec3(0.0);
    float accumulatedAlpha = 0.0;
    
    // Base position for cloud
    vec2 cloudUV = WorldPos.xz;
    
    for (int slice = 0; slice < 4; ++slice) {
        float sliceHeight = float(slice) / 4.0;
        float sliceDensity = SampleCloudDensity(cloudUV, sliceHeight);
        
        // Shape mask with coverage multiplier
        float mask = smoothstep(0.3, 0.7, sliceDensity);
        float alpha = mask * coverage * 0.25; // 0.25 per slice
        
        // Lighting from sun (angle-based)
        vec3 normal = vec3(0.0, 1.0, 0.0);
        float sunLight = clamp(dot(normal, normalize(sunDir)), 0.0, 1.0);
        
        // Soft shadow effect (density-based self-shadowing)
        float shadowFactor = 1.0 - sliceDensity * 0.3;
        sunLight *= shadowFactor;
        
        // Color based on lighting and height
        vec3 baseColor = mix(vec3(0.8, 0.82, 0.85), vec3(1.0, 0.98, 0.95), sunLight);
        baseColor = mix(baseColor, vec3(0.5, 0.55, 0.65), 1.0 - sunLight); // shadow color
        
        // Altitude tint
        vec3 altitudeTint = mix(vec3(0.7, 0.72, 0.75), vec3(1.0, 0.95, 0.9), sliceHeight);
        baseColor = mix(baseColor, altitudeTint, 0.3);
        
        // Front-to-back blending for volumetric effect
        accumulatedColor += (1.0 - accumulatedAlpha) * baseColor * alpha;
        accumulatedAlpha += (1.0 - accumulatedAlpha) * alpha;
        
        if (accumulatedAlpha > 0.95) break; // Early exit if opaque
    }
    
    // Edge fade to reduce visible tile seams
    vec2 texCoord = TexCoords;
    float edgeX = min(texCoord.x, 1.0 - texCoord.x);
    float edgeY = min(texCoord.y, 1.0 - texCoord.y);
    float edgeDist = min(edgeX, edgeY);
    float edgeFade = smoothstep(0.0, 0.1, edgeDist);
    
    accumulatedAlpha *= edgeFade;
    
    FragColor = vec4(accumulatedColor, accumulatedAlpha);
    if (FragColor.a < 0.01) discard;
}
