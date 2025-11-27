#ifndef VEGETATION_H
#define VEGETATION_H

#include <vector>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Terrain.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>

class Vegetation
{
public:
    Vegetation();
    ~Vegetation();

    void Generate(const Terrain &terrain, unsigned int grassCount = 1000, unsigned int treeCount = 100);
    void Render(Shader &shader);
    void RenderSnowOnTrees(Shader &shader, float snowAmount = 0.5f);
    bool LoadTreeModel(const std::string &modelPath);

    void SetSnowHideThreshold(float t) { hideThreshold = t; }

private:
    struct TreeInstance
    {
        glm::vec3 position;
        float scale;
    };

    unsigned int grassVAO, grassVBO, grassVertCount;
    unsigned int treeVAO, treeVBO, treeVertCount;
    unsigned int trunkVAO, trunkVBO, trunkVertCount;
    unsigned int modelVAO, modelVBO, modelEBO, modelVertCount; // For loaded models
    unsigned int instanceVBO;                                  // per-instance model matrices
    unsigned int instanceSeedVBO;                              // per-instance seed/variation
    unsigned int instanceCount;
    // Leaf card geometry (billboards)
    unsigned int leafVAO, leafVBO, leafVertCount;
    std::vector<glm::vec3> grassPositions;
    std::vector<TreeInstance> treeInstances;
    float hideThreshold; // snow depth threshold above which vegetation hides
    bool useLoadedModel; // Whether to use loaded model instead of procedural

    void InitRenderData();
    void GenerateConeMesh(std::vector<float> &vertices, float height, float baseRadius, int segments);
    void GenerateCylinderMesh(std::vector<float> &vertices, float height, float radius, int segments);
    void ProcessAssimpNode(aiNode *node, const aiScene *scene, std::vector<float> &vertices, std::vector<unsigned int> &indices);
    void ProcessAssimpMesh(aiMesh *mesh, const aiScene *scene, std::vector<float> &vertices, std::vector<unsigned int> &indices);

public:
    void RenderLeaves(Shader &leafShader, const class Camera &camera);
};
#endif
