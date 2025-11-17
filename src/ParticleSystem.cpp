#include "ParticleSystem.h"
#include "Terrain.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <algorithm>

ParticleSystem::ParticleSystem(unsigned int maxParticles)
    : maxParticles(maxParticles), emissionWidth(40.0f),
      emissionHeight(30.0f), emissionDepth(40.0f),
      windStrength(0.5f), wind(0.0f), paused(false), precipitationMode(PrecipitationMode::Snow),
      intensity(1.0f), particlesPerSecond(500.0f), terrain(nullptr), accumulatedTime(0.0f)
{
    particles.resize(maxParticles);
    InitRenderData();

    // Khởi tạo particles ngẫu nhiên
    for (unsigned int i = 0; i < maxParticles; ++i)
    {
        RespawnParticle(particles[i], glm::vec3(0.0f));
        particles[i].life = static_cast<float>(rand() % 100) / 100.0f;
    }
}

ParticleSystem::~ParticleSystem()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void ParticleSystem::Update(float deltaTime, const glm::vec3 &cameraPos)
{
    accumulatedTime += deltaTime;

    if (paused)
        return;

    // Spawn particles mới liên tục
    int newParticles = static_cast<int>(deltaTime * particlesPerSecond * intensity);

    for (int i = 0; i < newParticles; ++i)
    {
        unsigned int unusedParticle = FirstUnusedParticle();
        RespawnParticle(particles[unusedParticle], glm::vec3(0.0f));
    }

    // Cập nhật tất cả particles
    for (unsigned int i = 0; i < maxParticles; ++i)
    {
        Particle &p = particles[i];
        p.life -= deltaTime;

        if (p.life > 0.0f)
        {
            // Áp dụng trọng lực và gió (gồm vector wind chung và biến thể local)
            p.velocity.y -= 9.8f * p.weight * deltaTime;
            // global wind
            p.velocity += wind * deltaTime;
            // thêm biến thiên nhỏ theo vị trí để tạo hiệu ứng xoáy
            p.velocity.x += windStrength * sin(accumulatedTime * 2.0f + p.position.y * 0.1f) * deltaTime;
            p.velocity.z += windStrength * cos(accumulatedTime * 1.5f + p.position.x * 0.1f) * deltaTime * 0.5f;

            // Cập nhật vị trí
            p.position += p.velocity * deltaTime;

            // Xoay hạt tuyết
            p.rotation += p.rotationSpeed * deltaTime;

            // Kiểm tra va chạm với mặt đất (nếu có terrain thì dùng terrain->GetHeight)
            float groundY = 0.0f;
            if (terrain)
                groundY = terrain->GetHeight(p.position.x, p.position.z);

            if (p.position.y < groundY + 0.5f)
            {
                float fadeDistance = 0.5f;
                p.color.a = glm::clamp((p.position.y - groundY) / fadeDistance, 0.0f, 1.0f);

                // Khi chạm đất, xử lý phụ thuộc chế độ khí hậu (snow/rain)
                if (p.position.y <= groundY)
                {
                    if (precipitationMode == PrecipitationMode::Snow || precipitationMode == PrecipitationMode::Mix)
                    {
                        if (terrain)
                        {
                            // Thêm tuyết với lượng tùy thuộc kích thước và trọng lượng
                            float amount = p.size * 0.02f * (1.0f / (1.0f + p.weight));
                            // Đặt vị trí chính xác lên trên bề mặt để tránh xuyên qua terrain
                            float groundYExact = terrain->GetHeight(p.position.x, p.position.z);
                            glm::vec3 snowPos = glm::vec3(p.position.x, groundYExact, p.position.z);
                            terrain->AddSnow(snowPos, amount);
                            // Đặt lại y của particle để không xuyên xuống
                            p.position.y = groundYExact + 0.01f;
                        }
                    }
                    // Rain không tích tụ, chỉ mất particle
                    p.life = 0.0f;
                }
            }

            // Giới hạn phạm vi di chuyển
            if (glm::abs(p.position.x - cameraPos.x) > emissionWidth ||
                glm::abs(p.position.z - cameraPos.z) > emissionDepth)
            {
                p.life = 0.0f;
            }
        }
    }
}

void ParticleSystem::Render(Shader &shader, const Camera &camera)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    shader.use();

    // Sắp xếp particles theo khoảng cách từ camera (painter's algorithm)
    std::vector<std::pair<float, unsigned int>> sorted;
    for (unsigned int i = 0; i < maxParticles; ++i)
    {
        if (particles[i].life > 0.0f)
        {
            float distance = glm::length(camera.Position - particles[i].position);
            sorted.push_back(std::make_pair(distance, i));
        }
    }
    std::sort(sorted.begin(), sorted.end(),
              [](const std::pair<float, unsigned int> &a, const std::pair<float, unsigned int> &b)
              {
                  return a.first > b.first;
              });

    glBindVertexArray(VAO);

    for (auto &pair : sorted)
    {
        Particle &p = particles[pair.second];

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, p.position);

        // Billboard effect - particle luôn quay về phía camera
        glm::vec3 cameraRight = camera.Right;
        glm::vec3 cameraUp = camera.Up;

        model[0] = glm::vec4(cameraRight * p.size, 0.0f);
        model[1] = glm::vec4(cameraUp * p.size, 0.0f);
        model[2] = glm::vec4(glm::normalize(glm::cross(cameraRight, cameraUp)), 0.0f);

        model = glm::rotate(model, p.rotation, glm::vec3(0.0f, 0.0f, 1.0f));

        shader.setMat4("model", model);
        shader.setVec4("color", p.color);
        shader.setFloat("rotation", p.rotation);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void ParticleSystem::SetEmissionArea(float width, float height, float depth)
{
    emissionWidth = width;
    emissionHeight = height;
    emissionDepth = depth;
}

void ParticleSystem::SetWindStrength(float strength)
{
    windStrength = strength;
}

void ParticleSystem::SetWind(const glm::vec3 &w)
{
    wind = w;
}

void ParticleSystem::AddWind(const glm::vec3 &dw)
{
    wind += dw;
}

void ParticleSystem::TogglePause()
{
    paused = !paused;
}

void ParticleSystem::SetPrecipitationMode(ParticleSystem::PrecipitationMode mode)
{
    precipitationMode = mode;
}

void ParticleSystem::SetIntensity(float i)
{
    intensity = glm::max(0.0f, i);
}

void ParticleSystem::SetParticlesPerSecond(float pps)
{
    particlesPerSecond = glm::max(0.0f, pps);
}

void ParticleSystem::SetTerrain(Terrain *t)
{
    terrain = t;
}

void ParticleSystem::InitRenderData()
{
    // Quad vertices cho billboard
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ParticleSystem::RespawnParticle(Particle &particle, const glm::vec3 &offset)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> randX(-emissionWidth / 2, emissionWidth / 2);
    static std::uniform_real_distribution<float> randZ(-emissionDepth / 2, emissionDepth / 2);
    static std::uniform_real_distribution<float> randSize(0.05f, 0.2f);
    static std::uniform_real_distribution<float> randWeight(0.5f, 1.5f);
    static std::uniform_real_distribution<float> randRotSpeed(-2.0f, 2.0f);
    static std::uniform_real_distribution<float> randLife(8.0f, 15.0f);
    static std::uniform_real_distribution<float> randAlpha(0.7f, 1.0f);

    particle.position = glm::vec3(randX(gen), emissionHeight, randZ(gen)) + offset;
    // Behavior depends on precipitation mode
    if (precipitationMode == PrecipitationMode::Rain)
    {
        particle.velocity = glm::vec3(0.0f, -6.0f - randWeight(gen) * 2.0f, 0.0f);
        particle.color = glm::vec4(0.7f, 0.8f, 0.95f, 0.9f);
        particle.size = randSize(gen) * 0.4f;
        particle.life = randLife(gen) * 0.4f;
        particle.rotation = 0.0f;
        particle.rotationSpeed = 0.0f;
        particle.weight = randWeight(gen);
    }
    else if (precipitationMode == PrecipitationMode::Mix)
    {
        // Randomly choose rain or snow for this particle
        if ((rand() % 100) < 60)
        {
            // snow-like
            particle.velocity = glm::vec3(0.0f, -0.6f - randWeight(gen) * 0.3f, 0.0f);
            particle.color = glm::vec4(1.0f, 1.0f, 1.0f, randAlpha(gen));
            particle.size = randSize(gen) * 1.0f;
            particle.life = randLife(gen);
            particle.rotation = 0.0f;
            particle.weight = randWeight(gen);
        }
        else
        {
            // rain-like
            particle.velocity = glm::vec3(0.0f, -5.0f - randWeight(gen) * 2.0f, 0.0f);
            particle.color = glm::vec4(0.7f, 0.8f, 0.95f, 0.9f);
            particle.size = randSize(gen) * 0.4f;
            particle.life = randLife(gen) * 0.4f;
            particle.rotation = 0.0f;
            particle.rotationSpeed = 0.0f;
            particle.weight = randWeight(gen);
        }
    }
    else
    {
        // Snow default
        particle.velocity = glm::vec3(0.0f, -0.5f - randWeight(gen) * 0.2f, 0.0f);
        particle.color = glm::vec4(1.0f, 1.0f, 1.0f, randAlpha(gen));
        particle.size = randSize(gen);
        particle.life = randLife(gen);
        particle.rotation = 0.0f;
        particle.rotationSpeed = randRotSpeed(gen);
        particle.weight = randWeight(gen);
    }
}

unsigned int ParticleSystem::FirstUnusedParticle()
{
    static unsigned int lastUsedParticle = 0;

    for (unsigned int i = lastUsedParticle; i < maxParticles; ++i)
    {
        if (particles[i].life <= 0.0f)
        {
            lastUsedParticle = i;
            return i;
        }
    }

    for (unsigned int i = 0; i < lastUsedParticle; ++i)
    {
        if (particles[i].life <= 0.0f)
        {
            lastUsedParticle = i;
            return i;
        }
    }

    lastUsedParticle = 0;
    return 0;
}

unsigned int ParticleSystem::GetActiveParticleCount() const
{
    unsigned int count = 0;
    for (const auto &p : particles)
        if (p.life > 0.0f)
            ++count;
    return count;
}
float ParticleSystem::GetIntensity() const
{
    return intensity;
}