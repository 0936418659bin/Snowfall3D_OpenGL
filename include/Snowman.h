#ifndef SNOWMAN_H
#define SNOWMAN_H

#include <glm/glm.hpp>
#include "Shader.h"
#include "Terrain.h"

class Snowman
{
public:
    Snowman();
    ~Snowman();

    void SetPosition(const glm::vec3 &pos) { position = pos; }
    void SetTerrain(Terrain *t) { terrain = t; }
    void Render(Shader &shader);

private:
    glm::vec3 position;
    unsigned int sphereVAO, sphereVBO, sphereEBO;
    int indexCount;
    Terrain *terrain;

    void InitSphere(int latSegments = 12, int longSegments = 12);
    void DrawSphere(const glm::mat4 &model, Shader &shader);
};

#endif
