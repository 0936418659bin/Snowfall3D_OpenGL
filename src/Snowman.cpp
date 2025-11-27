#include "Snowman.h"
#include <glad/glad.h>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

Snowman::Snowman() : position(0.0f), sphereVAO(0), sphereVBO(0), sphereEBO(0), indexCount(0), terrain(nullptr)
{
    InitSphere(12, 12);
}

Snowman::~Snowman()
{
    if (sphereVAO)
        glDeleteVertexArrays(1, &sphereVAO);
    if (sphereVBO)
        glDeleteBuffers(1, &sphereVBO);
    if (sphereEBO)
        glDeleteBuffers(1, &sphereEBO);
}

void Snowman::InitSphere(int latSegments, int longSegments)
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int y = 0; y <= latSegments; ++y)
    {
        for (int x = 0; x <= longSegments; ++x)
        {
            float xSegment = (float)x / (float)longSegments;
            float ySegment = (float)y / (float)latSegments;
            float xPos = std::cos(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());
            float yPos = std::cos(ySegment * glm::pi<float>());
            float zPos = std::sin(xSegment * 2.0f * glm::pi<float>()) * std::sin(ySegment * glm::pi<float>());

            // position
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
            // normal
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
            // texcoord
            vertices.push_back(xSegment);
            vertices.push_back(ySegment);
        }
    }

    bool oddRow = false;
    for (int y = 0; y < latSegments; ++y)
    {
        for (int x = 0; x < longSegments; ++x)
        {
            int a = y * (longSegments + 1) + x;
            int b = a + longSegments + 1;
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(a + 1);

            indices.push_back(a + 1);
            indices.push_back(b);
            indices.push_back(b + 1);
        }
    }

    indexCount = static_cast<int>(indices.size());

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    GLsizei stride = 8 * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void Snowman::DrawSphere(const glm::mat4 &model, Shader &shader, const glm::vec3 &color)
{
    shader.setMat4("model", model);
    shader.setVec3("objectColor", color);
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Snowman::Render(Shader &shader)
{
    // Position on terrain if provided
    glm::vec3 pos = position;
    if (terrain)
    {
        float h = terrain->GetHeight(position.x, position.z);
        pos.y = h + 0.01f;
    }

    // Base sphere (white)
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, glm::vec3(1.2f));
    DrawSphere(model, shader, glm::vec3(1.0f, 1.0f, 1.0f));

    // Middle sphere (white)
    model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y + 1.1f, pos.z));
    model = glm::scale(model, glm::vec3(0.8f));
    DrawSphere(model, shader, glm::vec3(1.0f, 1.0f, 1.0f));

    // Scarf (orange torus around middle sphere) - simulate with a ring of small spheres
    float scarfY = pos.y + 1.1f;
    float scarfRadius = 0.95f; // slightly beyond middle sphere radius
    int scarfBeads = 8;
    for (int i = 0; i < scarfBeads; ++i)
    {
        float angle = (float)i / scarfBeads * 6.28318f;
        float sx = scarfRadius * cos(angle);
        float sz = scarfRadius * sin(angle);
        model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x + sx, scarfY, pos.z + sz));
        model = glm::scale(model, glm::vec3(0.15f));
        DrawSphere(model, shader, glm::vec3(1.0f, 0.5f, 0.0f)); // orange
    }

    // Head sphere (white)
    model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y + 1.9f, pos.z));
    model = glm::scale(model, glm::vec3(0.5f));
    DrawSphere(model, shader, glm::vec3(1.0f, 1.0f, 1.0f));

    // Eyes (black spheres on head)
    float eyeY = pos.y + 2.1f;
    float eyeZ = pos.z + 0.35f; // slightly forward on head

    // Left eye
    model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - 0.12f, eyeY, eyeZ));
    model = glm::scale(model, glm::vec3(0.08f));
    DrawSphere(model, shader, glm::vec3(0.0f, 0.0f, 0.0f)); // black

    // Right eye
    model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x + 0.12f, eyeY, eyeZ));
    model = glm::scale(model, glm::vec3(0.08f));
    DrawSphere(model, shader, glm::vec3(0.0f, 0.0f, 0.0f)); // black

    // Nose (orange, carrot-like - use small cone shape with spheres)
    float noseY = pos.y + 2.0f;
    float noseCenterZ = pos.z + 0.38f;

    // Nose tip
    model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, noseY, noseCenterZ));
    model = glm::scale(model, glm::vec3(0.1f, 0.08f, 0.12f));
    DrawSphere(model, shader, glm::vec3(1.0f, 0.4f, 0.0f)); // orange

    // Mouth (5 black dots in arc pattern)
    float mouthY = pos.y + 1.87f;
    float mouthZ = pos.z + 0.38f;
    int mouthDots = 5;
    float mouthRadius = 0.15f;
    for (int i = 0; i < mouthDots; ++i)
    {
        float t = (float)i / (mouthDots - 1);
        float mx = (t - 0.5f) * 0.3f;
        float my = -0.15f + sin(t * 3.14159f) * 0.1f; // slight arc downward
        model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x + mx, mouthY + my, mouthZ));
        model = glm::scale(model, glm::vec3(0.05f));
        DrawSphere(model, shader, glm::vec3(0.0f, 0.0f, 0.0f)); // black
    }
}
