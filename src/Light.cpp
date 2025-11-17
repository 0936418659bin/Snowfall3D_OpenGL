#include "Light.h"

Light::Light() {
    // Setup directional light mặc định (mặt trời mùa đông)
    dirLight.direction = glm::vec3(-0.3f, -0.8f, -0.5f);
    dirLight.ambient = glm::vec3(0.3f, 0.35f, 0.4f);
    dirLight.diffuse = glm::vec3(0.6f, 0.65f, 0.7f);
    dirLight.specular = glm::vec3(0.8f, 0.85f, 0.9f);
}

void Light::SetupShaderLights(Shader& shader) {
    shader.use();
    
    // Directional light
    shader.setVec3("dirLight.direction", dirLight.direction);
    shader.setVec3("dirLight.ambient", dirLight.ambient);
    shader.setVec3("dirLight.diffuse", dirLight.diffuse);
    shader.setVec3("dirLight.specular", dirLight.specular);
    
    // Point lights
    shader.setInt("numPointLights", pointLights.size());
    for (size_t i = 0; i < pointLights.size() && i < 4; ++i) {
        std::string number = std::to_string(i);
        shader.setVec3("pointLights[" + number + "].position", pointLights[i].position);
        shader.setVec3("pointLights[" + number + "].ambient", pointLights[i].ambient);
        shader.setVec3("pointLights[" + number + "].diffuse", pointLights[i].diffuse);
        shader.setVec3("pointLights[" + number + "].specular", pointLights[i].specular);
        shader.setFloat("pointLights[" + number + "].constant", pointLights[i].constant);
        shader.setFloat("pointLights[" + number + "].linear", pointLights[i].linear);
        shader.setFloat("pointLights[" + number + "].quadratic", pointLights[i].quadratic);
    }
}

void Light::Update(float deltaTime) {
    // Có thể thêm animation cho lights ở đây
    // Ví dụ: thay đổi hướng ánh sáng theo thời gian
}

void Light::AddPointLight(const glm::vec3& position, const glm::vec3& color) {
    PointLight light;
    light.position = position;
    light.ambient = color * 0.2f;
    light.diffuse = color;
    light.specular = color;
    pointLights.push_back(light);
}