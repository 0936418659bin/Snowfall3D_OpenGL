#ifndef CLOUDSYSTEM_H
#define CLOUDSYSTEM_H

#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"
#include "Camera.h"

struct Cloud
{
    glm::vec3 position;
    float size;
    float speed;
    float brightness; // độ sáng tùy theo hướng mặt trời
};

class CloudSystem
{
public:
    CloudSystem(unsigned int count = 80);
    ~CloudSystem();

    void Update(float deltaTime, const glm::vec3 &wind);
    void Render(Shader &shader, const Camera &camera);

    void SetArea(float width, float height, float depth);
    void SetCoverage(float c) { coverage = glm::clamp(c, 0.0f, 1.0f); }
    void SetEnabled(bool e) { enabled = e; }
    bool IsEnabled() const { return enabled; }

private:
    std::vector<Cloud> clouds;
    unsigned int VAO, VBO;
    unsigned int quadVerticesCount;
    float areaW, areaH, areaD;
    bool enabled;
    // 0..1 coverage for continuous cloud layer
    float coverage;
    void InitRenderData();
    void RespawnCloud(Cloud &c);
    // timing
    float elapsedTime;
};

#endif
