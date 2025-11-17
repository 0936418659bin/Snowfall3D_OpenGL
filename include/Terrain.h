#ifndef TERRAIN_H
#define TERRAIN_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Shader.h"

class Terrain
{
public:
    Terrain(float width, float depth, int resolution);
    ~Terrain();

    void Render(Shader &shader);
    void Update(float deltaTime);
    void AddSnow(const glm::vec3 &position, float amount);
    float GetHeight(float x, float z) const;
    void SetMeltSpeed(float s) { snowMeltSpeed = s; }
    float GetMeltSpeed() const { return snowMeltSpeed; }
    // Approximate total snow volume (snow depth sum * cell area)
    float GetTotalSnowVolume() const;
    float GetWidth() const { return width; }
    float GetDepth() const { return depth; }

private:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> snowDepth; // Độ sâu tuyết tại mỗi vertex
    std::vector<float> meltTimer; // Thời gian tồn tại của từng mảng tuyết trước khi bắt đầu tan

    float width;
    float depth;
    int resolution;
    float maxSnowDepth;
    float snowMeltSpeed;
    float patchLifetime; // thời gian mặc định một mảng tuyết tồn tại trước khi bắt đầu tan

    void GenerateTerrain();
    void UpdateSnowLayer();
    float PerlinNoise(float x, float z) const;
    int GetVertexIndex(int x, int z) const;
};

#endif