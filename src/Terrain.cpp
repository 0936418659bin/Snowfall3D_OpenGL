#include "Terrain.h"
#include <cmath>
#include <algorithm>

Terrain::Terrain(float width, float depth, int resolution)
    : width(width), depth(depth), resolution(resolution),
      maxSnowDepth(0.5f), snowMeltSpeed(0.05f), patchLifetime(10.0f)
{
    snowDepth.resize(resolution * resolution, 0.0f);
    meltTimer.resize(resolution * resolution, 0.0f);
    GenerateTerrain();
}

Terrain::~Terrain()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Terrain::GenerateTerrain()
{
    vertices.clear();
    indices.clear();

    float stepX = width / (resolution - 1);
    float stepZ = depth / (resolution - 1);

    // Tạo vertices với Perlin noise
    for (int z = 0; z < resolution; ++z)
    {
        for (int x = 0; x < resolution; ++x)
        {
            float posX = -width / 2.0f + x * stepX;
            float posZ = -depth / 2.0f + z * stepZ;
            float posY = PerlinNoise(posX * 0.1f, posZ * 0.1f) * 3.0f;

            // Position
            vertices.push_back(posX);
            vertices.push_back(posY);
            vertices.push_back(posZ);

            // Normal (tính sau)
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);

            // Texture coords
            vertices.push_back((float)x / (resolution - 1));
            vertices.push_back((float)z / (resolution - 1));

            // Snow depth
            vertices.push_back(0.0f);
        }
    }

    // Tính normals
    for (int z = 0; z < resolution - 1; ++z)
    {
        for (int x = 0; x < resolution - 1; ++x)
        {
            int i0 = z * resolution + x;
            int i1 = z * resolution + (x + 1);
            int i2 = (z + 1) * resolution + x;
            int i3 = (z + 1) * resolution + (x + 1);

            glm::vec3 v0(vertices[i0 * 9], vertices[i0 * 9 + 1], vertices[i0 * 9 + 2]);
            glm::vec3 v1(vertices[i1 * 9], vertices[i1 * 9 + 1], vertices[i1 * 9 + 2]);
            glm::vec3 v2(vertices[i2 * 9], vertices[i2 * 9 + 1], vertices[i2 * 9 + 2]);
            glm::vec3 v3(vertices[i3 * 9], vertices[i3 * 9 + 1], vertices[i3 * 9 + 2]);

            glm::vec3 normal1 = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            glm::vec3 normal2 = glm::normalize(glm::cross(v3 - v1, v2 - v1));

            // Cộng dồn normals
            for (int idx : {i0, i1, i2})
            {
                vertices[idx * 9 + 3] += normal1.x;
                vertices[idx * 9 + 4] += normal1.y;
                vertices[idx * 9 + 5] += normal1.z;
            }
            for (int idx : {i1, i2, i3})
            {
                vertices[idx * 9 + 3] += normal2.x;
                vertices[idx * 9 + 4] += normal2.y;
                vertices[idx * 9 + 5] += normal2.z;
            }
        }
    }

    // Normalize normals
    for (int i = 0; i < resolution * resolution; ++i)
    {
        glm::vec3 normal(vertices[i * 9 + 3], vertices[i * 9 + 4], vertices[i * 9 + 5]);
        normal = glm::normalize(normal);
        vertices[i * 9 + 3] = normal.x;
        vertices[i * 9 + 4] = normal.y;
        vertices[i * 9 + 5] = normal.z;
    }

    // Tạo indices
    for (int z = 0; z < resolution - 1; ++z)
    {
        for (int x = 0; x < resolution - 1; ++x)
        {
            int topLeft = z * resolution + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * resolution + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    // Setup OpenGL buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
    // TexCoord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(6 * sizeof(float)));
    // Snow depth
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(8 * sizeof(float)));

    glBindVertexArray(0);
}

void Terrain::Render(Shader &shader)
{
    shader.use();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Terrain::Update(float deltaTime)
{
    UpdateSnowLayer();

    // Giảm timer mảng tuyết; chỉ tan khi timer <= 0
    for (int i = 0; i < snowDepth.size(); ++i)
    {
        if (meltTimer[i] > 0.0f)
        {
            meltTimer[i] = std::max(0.0f, meltTimer[i] - deltaTime);
        }
        else
        {
            if (snowDepth[i] > 0.0f)
            {
                snowDepth[i] = std::max(0.0f, snowDepth[i] - snowMeltSpeed * deltaTime);
                vertices[i * 9 + 8] = snowDepth[i];
            }
        }
    }

    // Cập nhật VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Terrain::AddSnow(const glm::vec3 &position, float amount)
{
    int x = static_cast<int>((position.x + width / 2.0f) / width * (resolution - 1));
    int z = static_cast<int>((position.z + depth / 2.0f) / depth * (resolution - 1));

    if (x >= 0 && x < resolution && z >= 0 && z < resolution)
    {
        int idx = z * resolution + x;
        snowDepth[idx] = std::min(maxSnowDepth, snowDepth[idx] + amount);
        // Khi thêm tuyết, đặt timer cho ô này để tuyết tồn tại một khoảng trước khi bắt đầu tan
        meltTimer[idx] = std::max(meltTimer[idx], patchLifetime);

        // Lan tỏa tuyết sang các ô xung quanh
        for (int dz = -1; dz <= 1; ++dz)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                int nx = x + dx;
                int nz = z + dz;
                if (nx >= 0 && nx < resolution && nz >= 0 && nz < resolution)
                {
                    int nidx = nz * resolution + nx;
                    float distance = std::sqrt(dx * dx + dz * dz);
                    float falloff = 1.0f / (1.0f + distance);
                    snowDepth[nidx] = std::min(maxSnowDepth, snowDepth[nidx] + amount * falloff * 0.3f);
                    // Truyền một phần thời gian tồn tại sang ô lân cận
                    meltTimer[nidx] = std::max(meltTimer[nidx], patchLifetime * 0.5f);
                }
            }
        }
    }
}

float Terrain::GetHeight(float x, float z) const
{
    int gridX = static_cast<int>((x + width / 2.0f) / width * (resolution - 1));
    int gridZ = static_cast<int>((z + depth / 2.0f) / depth * (resolution - 1));

    if (gridX >= 0 && gridX < resolution - 1 && gridZ >= 0 && gridZ < resolution - 1)
    {
        int idx = gridZ * resolution + gridX;
        return vertices[idx * 9 + 1] + snowDepth[idx];
    }
    return 0.0f;
}

void Terrain::UpdateSnowLayer()
{
    // Cập nhật vertex data với snow depth mới
    for (int i = 0; i < snowDepth.size(); ++i)
    {
        vertices[i * 9 + 8] = snowDepth[i];
    }
}

float Terrain::PerlinNoise(float x, float z) const
{
    // Simplified Perlin noise với nhiều octaves
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < 4; ++i)
    {
        total += amplitude * (std::sin(x * frequency) * std::cos(z * frequency) +
                              std::sin((x + 100.0f) * frequency * 0.7f) * std::cos((z + 100.0f) * frequency * 0.7f));
        maxValue += amplitude;
        amplitude *= 0.5f;
        frequency *= 2.0f;
    }

    return total / maxValue;
}

int Terrain::GetVertexIndex(int x, int z) const
{
    return z * resolution + x;
}

float Terrain::GetTotalSnowVolume() const
{
    // Each vertex represents area ~ (width/(resolution-1)) * (depth/(resolution-1))
    float cellArea = (width / (resolution - 1)) * (depth / (resolution - 1));
    float total = 0.0f;
    for (float d : snowDepth)
        total += d * cellArea;
    return total;
}