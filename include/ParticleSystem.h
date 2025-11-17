#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Particle.h"
#include "Shader.h"
#include "Camera.h"
class Terrain;

class ParticleSystem
{
public:
    ParticleSystem(unsigned int maxParticles);
    ~ParticleSystem();

    enum class PrecipitationMode
    {
        Snow,
        Rain,
        Mix
    };

    void Update(float deltaTime, const glm::vec3 &cameraPos);
    void Render(Shader &shader, const Camera &camera);
    void SetEmissionArea(float width, float height, float depth);
    void SetWindStrength(float strength);
    void SetWind(const glm::vec3 &w);
    void AddWind(const glm::vec3 &dw);
    void TogglePause();
    void SetPrecipitationMode(PrecipitationMode mode);
    void SetIntensity(float intensity);
    void SetParticlesPerSecond(float pps);
    void SetTerrain(Terrain *t);
    unsigned int GetActiveParticleCount() const;
    glm::vec3 GetWind() const { return wind; }
    float GetIntensity() const;
    float GetParticlesPerSecond() const { return particlesPerSecond; }
    PrecipitationMode GetPrecipitationMode() const { return precipitationMode; }

private:
    std::vector<Particle> particles;
    unsigned int maxParticles;
    unsigned int VAO, VBO;
    float emissionWidth;
    float emissionHeight;
    float emissionDepth;
    float windStrength;
    glm::vec3 wind;
    bool paused;
    PrecipitationMode precipitationMode;
    float intensity; // multiplier for spawn rate
    float particlesPerSecond;
    Terrain *terrain;
    float accumulatedTime;

    void InitRenderData();
    void RespawnParticle(Particle &particle, const glm::vec3 &offset = glm::vec3(0.0f));
    unsigned int FirstUnusedParticle();
};

#endif