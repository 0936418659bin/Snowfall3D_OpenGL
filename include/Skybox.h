#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "Shader.h"

class Skybox
{
public:
    Skybox();
    ~Skybox();

    void Render(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection);
    void SetColor(const glm::vec3 &topColor, const glm::vec3 &horizonColor);
    void SetTimeOfDay(float hours); // 0-24
    void AdvanceTime(float hours);
    float GetTimeOfDay() const { return timeOfDay; }
    glm::vec3 GetSunDirection() const;

private:
    unsigned int VAO, VBO;
    glm::vec3 topColor;
    glm::vec3 horizonColor;
    float timeOfDay; // hours 0..24

    void InitSkybox();
};

#endif