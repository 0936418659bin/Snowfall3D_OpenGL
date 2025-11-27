#include "Vegetation.h"
#include "Camera.h"
#include <glad/glad.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

Vegetation::Vegetation() : grassVAO(0), grassVBO(0), grassVertCount(0),
                           treeVAO(0), treeVBO(0), treeVertCount(0),
                           trunkVAO(0), trunkVBO(0), trunkVertCount(0),
                           modelVAO(0), modelVBO(0), modelEBO(0), modelVertCount(0),
                           instanceVBO(0),
                           instanceSeedVBO(0),
                           instanceCount(0),
                           hideThreshold(0.2f),
                           leafVAO(0), leafVBO(0), leafVertCount(0),
                           useLoadedModel(false)
{
    InitRenderData();
}

Vegetation::~Vegetation()
{
    if (grassVAO)
        glDeleteVertexArrays(1, &grassVAO);
    if (grassVBO)
        glDeleteBuffers(1, &grassVBO);
    if (treeVAO)
        glDeleteVertexArrays(1, &treeVAO);
    if (treeVBO)
        glDeleteBuffers(1, &treeVBO);
    if (trunkVAO)
        glDeleteVertexArrays(1, &trunkVAO);
    if (trunkVBO)
        glDeleteBuffers(1, &trunkVBO);
    if (instanceVBO)
        glDeleteBuffers(1, &instanceVBO);
    if (instanceSeedVBO)
        glDeleteBuffers(1, &instanceSeedVBO);
    if (leafVAO)
        glDeleteVertexArrays(1, &leafVAO);
    if (leafVBO)
        glDeleteBuffers(1, &leafVBO);
    if (modelVAO)
        glDeleteVertexArrays(1, &modelVAO);
    if (modelVBO)
        glDeleteBuffers(1, &modelVBO);
    if (modelEBO)
        glDeleteBuffers(1, &modelEBO);
}

void Vegetation::GenerateConeMesh(std::vector<float> &vertices, float height, float baseRadius, int segments)
{
    // Generate a realistic cone (tree foliage) pointing up
    // This creates a proper 3D cone mesh with triangulated sides
    float pi = 3.14159265f;

    // Center point at base
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    // Base circle vertices
    for (int i = 0; i <= segments; ++i)
    {
        float angle = (float)i / segments * 2.0f * pi;
        float x = cos(angle) * baseRadius;
        float z = sin(angle) * baseRadius;
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }

    // Apex vertex
    vertices.push_back(0.0f);
    vertices.push_back(height);
    vertices.push_back(0.0f);
}

void Vegetation::GenerateCylinderMesh(std::vector<float> &vertices, float height, float radius, int segments)
{
    // Generate a cylinder (tree trunk)
    float pi = 3.14159265f;

    // Bottom circle
    for (int i = 0; i <= segments; ++i)
    {
        float angle = (float)i / segments * 2.0f * pi;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }

    // Top circle
    for (int i = 0; i <= segments; ++i)
    {
        float angle = (float)i / segments * 2.0f * pi;
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;
        vertices.push_back(x);
        vertices.push_back(height);
        vertices.push_back(z);
    }
}

void Vegetation::InitRenderData()
{
    // Grass billboard (crossed quads for dense grass look)
    float grassCross[] = {
        -0.15f, 0.0f, -0.15f, 0.15f, 0.0f, 0.15f, 0.0f, 0.8f, 0.0f,
        0.15f, 0.0f, -0.15f, -0.15f, 0.0f, 0.15f, 0.0f, 0.8f, 0.0f};
    grassVertCount = 6;
    glGenVertexArrays(1, &grassVAO);
    glGenBuffers(1, &grassVBO);
    glBindVertexArray(grassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grassCross), grassCross, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);

    // Create realistic procedural branching tree with trunk and radiating branches
    std::vector<float> treeVerts;
    float pi = 3.14159265f;

    // Main trunk - tapered cylinder
    float trunkH = 0.8f;
    for (int i = 0; i <= 12; ++i)
    {
        float a = (float)i / 12.0f * 2.0f * pi;
        treeVerts.push_back(sin(a) * 0.1f); // base
        treeVerts.push_back(0.0f);
        treeVerts.push_back(cos(a) * 0.1f);
        treeVerts.push_back(sin(a) * 0.05f); // top
        treeVerts.push_back(trunkH);
        treeVerts.push_back(cos(a) * 0.05f);
    }

    // Branch clusters at different heights
    for (int level = 0; level < 5; ++level)
    {
        float h = 0.15f + level * 0.15f;
        int nBranch = 4 + level * 2;
        float blen = 0.5f - level * 0.08f;
        float brad = 0.03f - level * 0.004f;

        for (int b = 0; b < nBranch; ++b)
        {
            float ba = (float)b / nBranch * 2.0f * pi;
            float bp = 0.35f + level * 0.05f;
            float bx = sin(ba) * cos(bp);
            float by = sin(bp);
            float bz = cos(ba) * cos(bp);

            for (int j = 0; j < 3; ++j)
            {
                float t = (float)j / 2.0f;
                float r = brad * (1.0f - t * 0.6f);
                float px = bx * blen * t;
                float py = h + by * blen * t;
                float pz = bz * blen * t;
                for (int c = 0; c <= 6; ++c)
                {
                    float ca = (float)c / 6.0f * 2.0f * pi;
                    treeVerts.push_back(px + sin(ca) * r);
                    treeVerts.push_back(py);
                    treeVerts.push_back(pz + cos(ca) * r);
                }
            }
        }
    }

    // Canopy silhouette
    for (int i = 0; i < 20; ++i)
    {
        float phi = (float)i / 20.0f * 2.0f * pi;
        float h = 0.2f + sin(phi * 3.0f) * 0.25f;
        float r = 0.35f + sin(phi * 2.0f) * 0.12f;
        treeVerts.push_back(sin(phi) * r);
        treeVerts.push_back(h);
        treeVerts.push_back(cos(phi) * r);
    }

    treeVertCount = (unsigned int)(treeVerts.size() / 3);
    glGenVertexArrays(1, &treeVAO);
    glGenBuffers(1, &treeVBO);
    glBindVertexArray(treeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glBufferData(GL_ARRAY_BUFFER, treeVerts.size() * sizeof(float), treeVerts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);

    // Tree trunk - tapered realistic bark
    std::vector<float> trunkVerts;
    int trunkSegments = 16;
    float trunkHeight = 0.7f;
    float trunkBaseRadius = 0.1f;

    // Bottom base (thicker)
    for (int i = 0; i <= trunkSegments; ++i)
    {
        float angle = (float)i / trunkSegments * 2.0f * pi;
        float x = cos(angle) * trunkBaseRadius;
        float z = sin(angle) * trunkBaseRadius;
        trunkVerts.push_back(x);
        trunkVerts.push_back(0.0f);
        trunkVerts.push_back(z);
    }

    // Center bottom
    trunkVerts.push_back(0.0f);
    trunkVerts.push_back(0.0f);
    trunkVerts.push_back(0.0f);

    // Middle section (medium thickness)
    float midTrunkRadius = trunkBaseRadius * 0.85f;
    for (int i = 0; i <= trunkSegments; ++i)
    {
        float angle = (float)i / trunkSegments * 2.0f * pi;
        float x = cos(angle) * midTrunkRadius;
        float z = sin(angle) * midTrunkRadius;
        trunkVerts.push_back(x);
        trunkVerts.push_back(trunkHeight * 0.5f);
        trunkVerts.push_back(z);
    }

    // Top section (tapered)
    float topTrunkRadius = trunkBaseRadius * 0.6f;
    for (int i = 0; i <= trunkSegments; ++i)
    {
        float angle = (float)i / trunkSegments * 2.0f * pi;
        float x = cos(angle) * topTrunkRadius;
        float z = sin(angle) * topTrunkRadius;
        trunkVerts.push_back(x);
        trunkVerts.push_back(trunkHeight);
        trunkVerts.push_back(z);
    }

    trunkVertCount = (unsigned int)(trunkVerts.size() / 3);
    glGenVertexArrays(1, &trunkVAO);
    glGenBuffers(1, &trunkVBO);
    glBindVertexArray(trunkVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trunkVBO);
    glBufferData(GL_ARRAY_BUFFER, trunkVerts.size() * sizeof(float), trunkVerts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);

    // Leaf card (a single quad centered at origin, will be billboarded in shader)
    float leafQuad[] = {
        -0.35f, 0.0f, 0.0f, 0.0f,
        0.35f, 0.0f, 1.0f, 0.0f,
        0.35f, 1.0f, 1.0f, 1.0f,

        -0.35f, 0.0f, 0.0f, 0.0f,
        0.35f, 1.0f, 1.0f, 1.0f,
        -0.35f, 1.0f, 0.0f, 1.0f};
    leafVertCount = 6;
    glGenVertexArrays(1, &leafVAO);
    glGenBuffers(1, &leafVBO);
    glBindVertexArray(leafVAO);
    glBindBuffer(GL_ARRAY_BUFFER, leafVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(leafQuad), leafQuad, GL_STATIC_DRAW);
    // pos.xy and uv.xy
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void Vegetation::Generate(const Terrain &terrain, unsigned int grassCount, unsigned int treeCount)
{
    grassPositions.clear();
    treeInstances.clear();

    float width = terrain.GetWidth();
    float depth = terrain.GetDepth();

    for (unsigned int i = 0; i < grassCount; ++i)
    {
        float x = ((float)rand() / RAND_MAX - 0.5f) * width;
        float z = ((float)rand() / RAND_MAX - 0.5f) * depth;
        float y = terrain.GetHeight(x, z);
        grassPositions.push_back(glm::vec3(x, y, z));
    }

    for (unsigned int i = 0; i < treeCount; ++i)
    {
        float x = ((float)rand() / RAND_MAX - 0.5f) * width;
        float z = ((float)rand() / RAND_MAX - 0.5f) * depth;
        float y = terrain.GetHeight(x, z);
        // Create a distribution of tree sizes: short, medium, tall
        float r = (float)rand() / RAND_MAX;
        float scale = 1.0f;
        if (r < 0.20f)
        {
            // Tall tree (20%): 2.0 - 3.5
            scale = 2.0f + ((float)rand() / RAND_MAX) * 1.5f;
        }
        else if (r < 0.65f)
        {
            // Medium tree (45%): 1.0 - 2.0
            scale = 1.0f + ((float)rand() / RAND_MAX) * 1.0f;
        }
        else
        {
            // Short/bush (35%): 0.4 - 0.95
            scale = 0.4f + ((float)rand() / RAND_MAX) * 0.55f;
        }
        treeInstances.push_back({glm::vec3(x, y, z), scale});
    }

    // Build instance matrix buffer for instanced rendering
    std::vector<glm::mat4> models;
    models.reserve(treeInstances.size());
    for (auto &t : treeInstances)
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, t.position);
        // random yaw rotation
        float yaw = ((float)rand() / RAND_MAX) * 6.2831853f;
        m = glm::rotate(m, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, glm::vec3(t.scale));
        models.push_back(m);
    }

    if (!instanceVBO)
        glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, models.size() * sizeof(glm::mat4), models.data(), GL_STATIC_DRAW);

    // Per-instance random seed (float) for wind/variation
    std::vector<float> seeds;
    seeds.reserve(treeInstances.size());
    for (unsigned int i = 0; i < treeInstances.size(); ++i)
    {
        seeds.push_back(((float)rand() / RAND_MAX));
    }
    if (!instanceSeedVBO)
        glGenBuffers(1, &instanceSeedVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceSeedVBO);
    glBufferData(GL_ARRAY_BUFFER, seeds.size() * sizeof(float), seeds.data(), GL_STATIC_DRAW);

    // Setup instance attrib pointers for treeVAO
    glBindVertexArray(treeVAO);
    std::size_t vec4Size = sizeof(glm::vec4);
    // ensure instanceVBO is bound when assigning matrix attribute pointers
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    for (unsigned int i = 0; i < 4; ++i)
    {
        glEnableVertexAttribArray(4 + i);
        glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(i * vec4Size));
        glVertexAttribDivisor(4 + i, 1);
    }
    // bind seed attribute to location 8 (seed buffer)
    glEnableVertexAttribArray(8);
    glBindBuffer(GL_ARRAY_BUFFER, instanceSeedVBO);
    glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
    glVertexAttribDivisor(8, 1);
    glBindVertexArray(0);

    // Setup instance attrib pointers for trunkVAO
    glBindVertexArray(trunkVAO);
    // ensure instanceVBO is bound for matrix attributes on trunk VAO as well
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    for (unsigned int i = 0; i < 4; ++i)
    {
        glEnableVertexAttribArray(4 + i);
        glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(i * vec4Size));
        glVertexAttribDivisor(4 + i, 1);
    }
    // trunk also gets seed attribute at location 8
    glEnableVertexAttribArray(8);
    glBindBuffer(GL_ARRAY_BUFFER, instanceSeedVBO);
    glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
    glVertexAttribDivisor(8, 1);
    glBindVertexArray(0);

    instanceCount = (unsigned int)models.size();
    std::cout << "[Vegetation] Generated " << instanceCount << " tree instances." << std::endl;
}

void Vegetation::RenderLeaves(Shader &leafShader, const Camera &camera)
{
    if (instanceCount == 0)
        return;
    leafShader.use();
    // projection/view/time are set by caller (main). Only pass camera vectors here
    leafShader.setVec3("camRight", camera.Right);
    leafShader.setVec3("camUp", camera.Up);
    leafShader.setVec3("sunDir", glm::vec3(0.0f, 1.0f, 0.0f));

    // Bind instance matrices
    glBindVertexArray(leafVAO);
    // bind instance matrix buffer to attribute locations 4-7
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    std::size_t vec4Size = sizeof(glm::vec4);
    for (unsigned int i = 0; i < 4; ++i)
    {
        glEnableVertexAttribArray(4 + i);
        glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(i * vec4Size));
        glVertexAttribDivisor(4 + i, 1);
    }
    // seed attribute at 8
    glBindBuffer(GL_ARRAY_BUFFER, instanceSeedVBO);
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
    glVertexAttribDivisor(8, 1);

    // draw instanced leaf cards
    glDrawArraysInstanced(GL_TRIANGLES, 0, leafVertCount, instanceCount);
    glBindVertexArray(0);
}

void Vegetation::Render(Shader &shader)
{
    shader.use();

    // Render grass billboards (simple, non-instanced for now)
    for (auto &g : grassPositions)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, g);
        shader.setMat4("model", model);
        glBindVertexArray(grassVAO);
        glDrawArrays(GL_TRIANGLES, 0, grassVertCount);
    }

    // Render instanced trees (trunks and foliage)
    if (treeInstances.empty())
        return;

    // If loaded model exists, render it instead of procedural geometry
    if (useLoadedModel && modelVAO && modelVertCount > 0)
    {
        glBindVertexArray(modelVAO);
        shader.setVec3("objectColor", glm::vec3(0.6f, 0.5f, 0.3f));
        glDrawElementsInstanced(GL_TRIANGLES, modelVertCount, GL_UNSIGNED_INT, 0, (GLsizei)treeInstances.size());
        return;
    }

    // Debug: draw first tree non-instanced to verify geometry/placement
    {
        auto &t0 = treeInstances[0];
        glm::mat4 modelDbg = glm::mat4(1.0f);
        modelDbg = glm::translate(modelDbg, t0.position);
        modelDbg = glm::scale(modelDbg, glm::vec3(t0.scale));
        shader.setMat4("model", modelDbg);
        // draw trunk and first foliage section for debug
        shader.setVec3("objectColor", glm::vec3(0.6f, 0.4f, 0.25f));
        glBindVertexArray(trunkVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, trunkVertCount);
        shader.setVec3("objectColor", glm::vec3(0.15f, 0.55f, 0.18f));
        glBindVertexArray(treeVAO);
        unsigned int debugSection = glm::min((unsigned int)treeVertCount, 22u);
        glDrawArrays(GL_TRIANGLE_FAN, 0, debugSection);
    }

    // Render all tree trunks with instancing
    glBindVertexArray(trunkVAO);
    shader.setVec3("objectColor", glm::vec3(0.5f, 0.35f, 0.18f)); // Brown bark
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, trunkVertCount, (GLsizei)treeInstances.size());

    // Render all tree foliage with instancing
    glBindVertexArray(treeVAO);
    shader.setVec3("objectColor", glm::vec3(0.15f, 0.55f, 0.18f)); // Foliage green
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, treeVertCount, (GLsizei)treeInstances.size());
}

void Vegetation::ProcessAssimpMesh(aiMesh *mesh, const aiScene *scene, std::vector<float> &vertices, std::vector<unsigned int> &indices)
{
    // Process vertex positions
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }
}

void Vegetation::ProcessAssimpNode(aiNode *node, const aiScene *scene, std::vector<float> &vertices, std::vector<unsigned int> &indices)
{
    // Process all meshes at this node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessAssimpMesh(mesh, scene, vertices, indices);
    }

    // Process all children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessAssimpNode(node->mChildren[i], scene, vertices, indices);
    }
}

bool Vegetation::LoadTreeModel(const std::string &modelPath)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(modelPath,
                                             aiProcess_Triangulate |
                                                 aiProcess_FlipWindingOrder |
                                                 aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        return false;
    }

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Process all meshes from the model
    ProcessAssimpNode(scene->mRootNode, scene, vertices, indices);

    if (vertices.empty() || indices.empty())
    {
        std::cerr << "No mesh data loaded from model" << std::endl;
        return false;
    }

    modelVertCount = indices.size();

    // Setup model VAO/VBO/EBO
    if (!modelVAO)
        glGenVertexArrays(1, &modelVAO);
    if (!modelVBO)
        glGenBuffers(1, &modelVBO);
    if (!modelEBO)
        glGenBuffers(1, &modelEBO);

    glBindVertexArray(modelVAO);

    // VBO: vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // EBO: indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute (3 floats per vertex)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // Setup instance matrix attributes (4-7) for model too
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    for (int i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(4 + i);
        glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(4 + i, 1);
    }

    // Instance seed attribute (8)
    glBindBuffer(GL_ARRAY_BUFFER, instanceSeedVBO);
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
    glVertexAttribDivisor(8, 1);

    glBindVertexArray(0);

    useLoadedModel = true;
    std::cout << "Loaded tree model: " << modelPath << " (" << modelVertCount << " indices)" << std::endl;
    return true;
}

void Vegetation::RenderSnowOnTrees(Shader &shader, float snowAmount)
{
    // snowAmount: 0.0-1.0, represents how much snow has accumulated (0=none, 1=heavy)
    if (snowAmount < 0.01f || treeInstances.empty())
        return;

    shader.use();

    // Render a white cap/layer on each tree's foliage
    // For simplicity, we render semi-transparent spheres positioned on top of each tree
    if (treeInstances.empty())
        return;

    for (size_t i = 0; i < treeInstances.size(); ++i)
    {
        auto &t = treeInstances[i];

        // Draw a semi-transparent white sphere on top of foliage
        // Snow cap height increases with snowAmount
        float capHeight = 0.3f + snowAmount * 0.5f;
        float capY = t.position.y + 0.4f + snowAmount * 0.1f;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(t.position.x, capY, t.position.z));
        model = glm::scale(model, glm::vec3(t.scale * (0.5f + snowAmount * 0.2f)));
        shader.setMat4("model", model);
        shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f)); // white
    }
}
