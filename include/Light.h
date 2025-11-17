#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include "Shader.h"

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    
    DirectionalLight() 
        : direction(-0.3f, -0.8f, -0.5f),
          ambient(0.3f, 0.35f, 0.4f),
          diffuse(0.6f, 0.65f, 0.7f),
          specular(0.8f, 0.85f, 0.9f) {}
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    
    float constant;
    float linear;
    float quadratic;
    
    PointLight()
        : position(0.0f),
          ambient(0.2f),
          diffuse(0.8f),
          specular(1.0f),
          constant(1.0f),
          linear(0.09f),
          quadratic(0.032f) {}
};

class Light {
public:
    DirectionalLight dirLight;
    std::vector<PointLight> pointLights;
    
    Light();
    void SetupShaderLights(Shader& shader);
    void Update(float deltaTime);
    void AddPointLight(const glm::vec3& position, const glm::vec3& color);
};

#endif