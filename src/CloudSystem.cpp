#include "CloudSystem.h"
#include <glad/glad.h>
#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>

CloudSystem::CloudSystem(unsigned int count)
    : areaW(80.0f), areaH(25.0f), areaD(80.0f), enabled(true), elapsedTime(0.0f)
{
    clouds.resize(count);
    for (auto &c : clouds)
        RespawnCloud(c);

    InitRenderData();
    coverage = 0.6f;
}

CloudSystem::~CloudSystem()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void CloudSystem::SetArea(float width, float height, float depth)
{
    areaW = width;
    areaH = height;
    areaD = depth;
}

void CloudSystem::RespawnCloud(Cloud &c)
{
    float x = ((float)rand() / RAND_MAX - 0.5f) * areaW;
    float z = ((float)rand() / RAND_MAX - 0.5f) * areaD;
    float y = areaH * (0.55f + 0.45f * ((float)rand() / RAND_MAX));
    c.position = glm::vec3(x, y, z);
    c.size = 6.0f + ((float)rand() / RAND_MAX) * 12.0f;
    c.speed = 0.15f + ((float)rand() / RAND_MAX) * 0.6f;
    c.brightness = 1.0f;
}

void CloudSystem::InitRenderData()
{
    float quad[] = {
        -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f};

    quadVerticesCount = 6;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void CloudSystem::Update(float deltaTime, const glm::vec3 &wind)
{
    if (!enabled)
        return;
    elapsedTime += deltaTime;
    for (auto &c : clouds)
    {
        c.position += glm::vec3(wind.x * c.speed * deltaTime, 0.0f, wind.z * c.speed * deltaTime);
        // brightness based on altitude (higher clouds catch more light)
        float alt = glm::clamp((c.position.y / areaH), 0.0f, 1.0f);
        c.brightness = 0.7f + 0.6f * alt;

        if (c.position.x < -areaW / 2.0f - 15.0f)
            c.position.x = areaW / 2.0f + 15.0f;
        if (c.position.x > areaW / 2.0f + 15.0f)
            c.position.x = -areaW / 2.0f - 15.0f;
        if (c.position.z < -areaD / 2.0f - 15.0f)
            c.position.z = areaD / 2.0f + 15.0f;
        if (c.position.z > areaD / 2.0f + 15.0f)
            c.position.z = -areaD / 2.0f - 15.0f;
    }
}

void CloudSystem::Render(Shader &shader, const Camera &camera)
{
    if (!enabled)
        return;

    shader.use();
    // caller should set projection/view and sunDir/time uniforms
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Render a tiled continuous cloud layer across the areaW x areaD using a grid of large quads.
    // coverage controls overall opacity/density (0..1)
    shader.setFloat("coverage", coverage);
    int gridX = 24;
    int gridZ = 24;
    float cellW = areaW / (float)gridX;
    float cellD = areaD / (float)gridZ;
    float baseY = areaH * 0.7f; // cloud layer height

    for (int iz = 0; iz < gridZ; ++iz)
    {
        for (int ix = 0; ix < gridX; ++ix)
        {
            float x = -areaW / 2.0f + (ix + 0.5f) * cellW;
            float z = -areaD / 2.0f + (iz + 0.5f) * cellD;

            // each tile is slightly larger than cell to avoid seams (use overlap)
            float scaleX = cellW * 1.6f;
            float scaleY = cellD * 1.6f;

            // small spatial jitter per tile for variation
            float jitter = (float)(((ix + 1) * 73856093u) ^ ((iz + 1) * 19349663u)) * 0.00000005f;
            glm::vec3 pos = glm::vec3(x + jitter * 5.0f, baseY - (jitter * 8.0f), z + jitter * 5.0f);

            // Build a model matrix that orients the quad to face the camera (billboard)
            glm::vec3 camRight = camera.Right;
            glm::vec3 camUp = camera.Up;

            glm::mat4 model(1.0f);
            // First column = right * scaleX
            model[0] = glm::vec4(camRight * scaleX, 0.0f);
            // Second column = up * scaleY
            model[1] = glm::vec4(camUp * scaleY, 0.0f);
            // Third column - forward axis (not used for quad vertices but keep orthonormal)
            glm::vec3 forward = glm::normalize(glm::cross(camRight, camUp));
            model[2] = glm::vec4(forward, 0.0f);
            // Translation
            model[3] = glm::vec4(pos, 1.0f);

            shader.setMat4("model", model);
            shader.setFloat("time", elapsedTime);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, quadVerticesCount);
            glBindVertexArray(0);
        }
    }

    glDisable(GL_BLEND);
}
