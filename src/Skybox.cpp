#include "Skybox.h"

Skybox::Skybox()
    : topColor(0.5f, 0.6f, 0.7f), horizonColor(0.7f, 0.75f, 0.8f), timeOfDay(12.0f)
{
    InitSkybox();
}

Skybox::~Skybox()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Skybox::InitSkybox()
{
    float skyboxVertices[] = {
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    glBindVertexArray(0);
}

void Skybox::Render(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection)
{
    glDepthFunc(GL_LEQUAL);

    shader.use();
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
    shader.setMat4("view", viewNoTranslation);
    shader.setMat4("projection", projection);
    // Derive colors from timeOfDay (simple day/night blend)
    glm::vec3 dayTop(0.5f, 0.7f, 0.95f);
    glm::vec3 dayHorizon(0.9f, 0.8f, 0.6f);
    glm::vec3 nightTop(0.02f, 0.03f, 0.08f);
    glm::vec3 nightHorizon(0.1f, 0.05f, 0.15f);

    float t = 0.0f;
    // timeOfDay in [0,24)
    if (timeOfDay >= 6.0f && timeOfDay <= 18.0f)
    {
        // day -> between 6..18
        t = (timeOfDay - 6.0f) / 12.0f; // 0..1
    }
    else
    {
        // night
        if (timeOfDay < 6.0f)
            t = 0.0f;
        else
            t = 1.0f;
    }

    glm::vec3 curTop = glm::mix(nightTop, dayTop, t);
    glm::vec3 curHorizon = glm::mix(nightHorizon, dayHorizon, t);
    shader.setVec3("topColor", curTop);
    shader.setVec3("horizonColor", curHorizon);
    shader.setFloat("timeOfDay", timeOfDay);
    // compute canonical sun direction from timeOfDay (simple orbital path)
    float theta = (timeOfDay / 24.0f) * 2.0f * 3.14159265f; // 0..2pi
    // Put sun on a circle in X-Y plane with slight tilt, move along +X -> -X over day
    glm::vec3 sunDir = glm::normalize(glm::vec3(cos(theta), sin(theta), 0.15f));
    shader.setVec3("sunDirection", sunDir);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);
}

void Skybox::SetTimeOfDay(float hours)
{
    timeOfDay = fmod(hours, 24.0f);
    if (timeOfDay < 0.0f)
        timeOfDay += 24.0f;
}

void Skybox::AdvanceTime(float hours)
{
    SetTimeOfDay(timeOfDay + hours);
}

void Skybox::SetColor(const glm::vec3 &top, const glm::vec3 &horizon)
{
    topColor = top;
    horizonColor = horizon;
}

glm::vec3 Skybox::GetSunDirection() const
{
    float theta = (timeOfDay / 24.0f) * 2.0f * 3.14159265f;
    return glm::normalize(glm::vec3(cos(theta), sin(theta), 0.15f));
}