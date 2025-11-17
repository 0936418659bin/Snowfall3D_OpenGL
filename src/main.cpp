#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <cstdio>

#include "Shader.h"
#include "Camera.h"
#include "ParticleSystem.h"
#include "Terrain.h"
#include "Skybox.h"
#include "Light.h"
#include "CloudSystem.h"
#include "Snowman.h"
#include "Vegetation.h"

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
Camera camera(glm::vec3(0.0f, 5.0f, 15.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Global pointers for input callbacks
static class ParticleSystem *gParticleSystem = nullptr;
static class Terrain *gTerrain = nullptr;
static class Skybox *gSkybox = nullptr;
static class CloudSystem *gCloudSystem = nullptr;
static bool gShowStats = false;
// Time control
static bool gAutoTime = false;
static float gTimeSpeed = 0.1f; // hours per second when auto time enabled

// Callbacks
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // MSAA

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Snowfall 3D - Computer Graphics", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Build and compile shaders
    Shader particleShader("shaders/particle.vert", "shaders/particle.frag");
    Shader terrainShader("shaders/terrain.vert", "shaders/terrain.frag");
    Shader skyboxShader("shaders/skybox.vert", "shaders/skybox.frag");
    Shader cloudShader("shaders/cloud.vert", "shaders/cloud.frag");
    Shader vegetationShader("shaders/vegetation.vert", "shaders/vegetation.frag");
    Shader leafShader("shaders/leaf.vert", "shaders/leaf.frag");

    // Create objects
    ParticleSystem snowSystem(5000);
    gParticleSystem = &snowSystem;
    Terrain terrain(50.0f, 50.0f, 100);
    Skybox skybox;
    Light light;
    CloudSystem clouds(40);
    Snowman snowman;
    Vegetation vegetation;
    gCloudSystem = &clouds;
    gTerrain = &terrain;
    gSkybox = &skybox;

    // Hook terrain for snowman and vegetation
    snowman.SetTerrain(&terrain);
    snowman.SetPosition(glm::vec3(5.0f, 0.0f, -3.0f));
    vegetation.Generate(terrain, 1200, 180); // generate grass + trees (instanced)
    
    // Try to load tree model from Assimp
    // Prefer the runtime assets folder (relative to executable), then project assets
    if (!vegetation.LoadTreeModel("assets/tree.obj") &&
        !vegetation.LoadTreeModel("tree.obj") &&
        !vegetation.LoadTreeModel("../assets/tree.obj")) {
        std::cout << "Note: Tree model file not found. Using procedural trees." << std::endl;
    }

    // Setup lighting
    light.SetupShaderLights(terrainShader);

    // Configure particle system
    snowSystem.SetEmissionArea(40.0f, 25.0f, 40.0f);
    snowSystem.SetWindStrength(1.5f);
    snowSystem.SetTerrain(&terrain);
    snowSystem.SetWind(glm::vec3(1.0f, 0.0f, 0.0f)); // mặc định gió nhẹ về +X

    // Skybox colors (winter atmosphere)
    skybox.SetColor(glm::vec3(0.5f, 0.6f, 0.7f), glm::vec3(0.7f, 0.75f, 0.8f));
    skybox.SetTimeOfDay(10.5f);

    std::cout << "\n=== SNOWFALL 3D - COMPUTER GRAPHICS PROJECT ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  WASD - Move camera" << std::endl;
    std::cout << "  Mouse - Look around" << std::endl;
    std::cout << "  Space/LShift - Move up/down" << std::endl;
    std::cout << "  Scroll - Zoom in/out" << std::endl;
    std::cout << "  ESC - Exit\n"
              << std::endl;
    std::cout << "  P - Pause weather   R - Cycle precipitation (Snow/Mix/Rain)" << std::endl;
    std::cout << "  [ / ] - Intensity down/up    J / L - Wind left/right" << std::endl;
    std::cout << "  I / K - Increase/Decrease particle rate" << std::endl;
    std::cout << "  Z / X - Increase/Decrease snowfall (particles/sec)" << std::endl;
    std::cout << "  M / N - Increase/Decrease snow melt speed" << std::endl;
    std::cout << "  O - Toggle projection (Perspective/Orthographic)" << std::endl;
    std::cout << "  H - Toggle on-screen stats (window title)" << std::endl;
    std::cout << "  T - Advance time by +1 hour" << std::endl;
    std::cout << "  Y - Toggle day/night (12h / 0h)" << std::endl;
    std::cout << "  B - Toggle auto time progression" << std::endl;

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Update
        snowSystem.Update(deltaTime, camera.Position);
        terrain.Update(deltaTime);
        light.Update(deltaTime);
        // Advance sky time if auto enabled
        if (gAutoTime)
        {
            float advance = gTimeSpeed * deltaTime; // hours
            skybox.AdvanceTime(advance);
        }

        // Render
        glClearColor(0.5f, 0.6f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // View/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Render skybox
        skyboxShader.use();
        skyboxShader.setMat4("projection", projection);
        skybox.Render(skyboxShader, view, projection);

        // Render clouds with dedicated cloud shader
        cloudShader.use();
        cloudShader.setMat4("projection", projection);
        cloudShader.setMat4("view", view);
        // pass sun direction and time (skybox provides it)
        cloudShader.setVec3("sunDir", gSkybox ? gSkybox->GetSunDirection() : glm::vec3(0, 1, 0));
        cloudShader.setFloat("time", gSkybox ? gSkybox->GetTimeOfDay() : 0.0f);
        // Map particle system intensity -> cloud coverage. More intense precipitation => denser clouds.
        float intensity = snowSystem.GetIntensity();
        auto pm = snowSystem.GetPrecipitationMode();
        // baseline coverage from intensity (tuned empirically)
        float coverage = glm::clamp(intensity / 3.0f, 0.05f, 1.0f);
        if (pm == ParticleSystem::PrecipitationMode::Snow)
        {
            // snow mode makes clouds thicker
            coverage = glm::clamp(coverage * 1.2f, 0.0f, 1.0f);
        }
        else if (pm == ParticleSystem::PrecipitationMode::Mix)
        {
            coverage = glm::clamp(coverage * 0.9f, 0.0f, 1.0f);
        }
        else
        {
            // rain: lower cloud coverage for snowfall scenario
            coverage = glm::clamp(coverage * 0.6f, 0.0f, 1.0f);
        }
        clouds.SetCoverage(coverage);
        cloudShader.setFloat("coverage", coverage);
        clouds.Update(deltaTime, snowSystem.GetWind());
        clouds.Render(cloudShader, camera);

        // Render vegetation (instanced) and snowman
        vegetationShader.use();
        vegetationShader.setMat4("projection", projection);
        vegetationShader.setMat4("view", view);
        vegetationShader.setFloat("time", (float)glfwGetTime());
        vegetationShader.setVec3("sunDir", gSkybox ? gSkybox->GetSunDirection() : glm::vec3(0,1,0));
        // pass wind direction to vegetation shader for sway
        vegetationShader.setVec3("windDir", snowSystem.GetWind());
        // Set trunk/foliage colors and blend parameters
        vegetationShader.setVec3("trunkColor", glm::vec3(0.45f, 0.32f, 0.20f)); // brown
        vegetationShader.setVec3("foliageColor", glm::vec3(0.12f, 0.5f, 0.17f)); // green
        vegetationShader.setFloat("foliageStart", 0.6f);
        vegetationShader.setFloat("foliageBlend", 0.6f);
        vegetation.Render(vegetationShader);

        // Render leaf cards (billboarded quads) with separate shader
        leafShader.use();
        leafShader.setMat4("projection", projection);
        leafShader.setMat4("view", view);
        leafShader.setFloat("time", (float)glfwGetTime());
        leafShader.setVec3("sunDir", gSkybox ? gSkybox->GetSunDirection() : glm::vec3(0,1,0));
        leafShader.setVec3("camRight", camera.Right);
        leafShader.setVec3("camUp", camera.Up);
        vegetation.RenderLeaves(leafShader, camera);

        terrainShader.use();
        snowman.Render(terrainShader);

        // Render terrain
        terrainShader.use();
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);
        terrainShader.setVec3("viewPos", camera.Position);

        glm::mat4 model = glm::mat4(1.0f);
        terrainShader.setMat4("model", model);

        light.SetupShaderLights(terrainShader);
        terrain.Render(terrainShader);

        // Render particles
        particleShader.use();
        particleShader.setMat4("projection", projection);
        particleShader.setMat4("view", view);
        snowSystem.Render(particleShader, camera);

        // Update window title with on-screen stats if enabled
        if (gShowStats)
        {
            unsigned int active = gParticleSystem ? gParticleSystem->GetActiveParticleCount() : 0;
            float volume = gTerrain ? gTerrain->GetTotalSnowVolume() : 0.0f;
            float fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
            float timeOfDay = gSkybox ? gSkybox->GetTimeOfDay() : 0.0f;
            // Compose a short title (include time)
            char buf[256];
            int hrs = (int)timeOfDay;
            int mins = (int)((timeOfDay - hrs) * 60.0f);
            snprintf(buf, sizeof(buf), "Snowfall3D - Part:%u Vol:%dm3 FPS:%d Time:%02d:%02d", active, (int)volume, (int)fps, hrs, mins);
            glfwSetWindowTitle(window, buf);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

    // Extended particle / weather controls (edge-detect keys)
    static bool prevP = false, prevR = false, prevBracketL = false, prevBracketR = false;
    static bool prevJ = false, prevL = false, prevI = false, prevK = false;
    static bool prevZ = false, prevX = false, prevM = false, prevN = false, prevO = false, prevH = false, prevC = false;
    static bool prevT = false, prevY = false, prevB = false;

    bool curP = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
    bool curR = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    bool curBL = glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS;
    bool curBR = glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS;
    bool curJ = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
    bool curLL = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
    bool curI = glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS;
    bool curK = glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS;
    bool curZ = glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS;
    bool curX = glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS;
    bool curM = glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS;
    bool curN = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;
    bool curO = glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS;
    bool curH = glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS;
    bool curC = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    bool curT = glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS;
    bool curY = glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS;
    bool curB = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;

    // Toggle pause (P)
    if (curP && !prevP && gParticleSystem)
    {
        gParticleSystem->TogglePause();
        std::cout << "[Weather] Toggle pause.\n";
    }

    // Cycle precipitation (R)
    static int precipState = 0; // 0: Snow, 1: Mix, 2: Rain
    if (curR && !prevR && gParticleSystem)
    {
        precipState = (precipState + 1) % 3;
        if (precipState == 0)
            gParticleSystem->SetPrecipitationMode(ParticleSystem::PrecipitationMode::Snow);
        else if (precipState == 1)
            gParticleSystem->SetPrecipitationMode(ParticleSystem::PrecipitationMode::Mix);
        else
            gParticleSystem->SetPrecipitationMode(ParticleSystem::PrecipitationMode::Rain);
        std::cout << "[Weather] Precipitation set to " << (precipState == 0 ? "Snow" : (precipState == 1 ? "Mix" : "Rain")) << "\n";
    }

    // Decrease/increase intensity ([ and ]) - multiplies current intensity
    static float currentIntensity = 1.0f;
    if (curBL && !prevBracketL && gParticleSystem)
    {
        currentIntensity = glm::max(0.05f, currentIntensity * 0.7f);
        gParticleSystem->SetIntensity(currentIntensity);
        std::cout << "[Weather] Intensity decreased to " << currentIntensity << "\n";
    }
    if (curBR && !prevBracketR && gParticleSystem)
    {
        currentIntensity = glm::min(10.0f, currentIntensity * 1.3f);
        gParticleSystem->SetIntensity(currentIntensity);
        std::cout << "[Weather] Intensity increased to " << currentIntensity << "\n";
    }

    // Rotate/adjust wind direction (J / L) - apply small change to global wind
    if (curJ && !prevJ && gParticleSystem)
    {
        gParticleSystem->AddWind(glm::vec3(-1.0f, 0.0f, 0.0f));
        std::cout << "[Weather] Wind nudged left.\n";
    }
    if (curLL && !prevL && gParticleSystem)
    {
        gParticleSystem->AddWind(glm::vec3(1.0f, 0.0f, 0.0f));
        std::cout << "[Weather] Wind nudged right.\n";
    }

    // Increase/decrease particle rate (I / K)
    static float currentPPS = 500.0f;
    if (curI && !prevI && gParticleSystem)
    {
        currentPPS = glm::min(5000.0f, currentPPS * 1.5f);
        gParticleSystem->SetParticlesPerSecond(currentPPS);
        std::cout << "[Weather] Particles per second: " << currentPPS << "\n";
    }
    if (curK && !prevK && gParticleSystem)
    {
        currentPPS = glm::max(10.0f, currentPPS * 0.6f);
        gParticleSystem->SetParticlesPerSecond(currentPPS);
        std::cout << "[Weather] Particles per second: " << currentPPS << "\n";
    }

    // Z/X - quick increase/decrease particles per second
    if (curZ && !prevZ && gParticleSystem)
    {
        currentPPS = glm::min(20000.0f, currentPPS * 2.0f);
        gParticleSystem->SetParticlesPerSecond(currentPPS);
        std::cout << "[Weather] (Z) Particles per second: " << currentPPS << "\n";
    }
    if (curX && !prevX && gParticleSystem)
    {
        currentPPS = glm::max(10.0f, currentPPS * 0.5f);
        gParticleSystem->SetParticlesPerSecond(currentPPS);
        std::cout << "[Weather] (X) Particles per second: " << currentPPS << "\n";
    }

    // M/N - increase/decrease snow melt speed
    if (curM && !prevM && gTerrain)
    {
        float s = gTerrain->GetMeltSpeed();
        s = glm::min(5.0f, s * 1.5f);
        gTerrain->SetMeltSpeed(s);
        std::cout << "[Terrain] Melt speed increased to " << s << "\n";
    }
    if (curN && !prevN && gTerrain)
    {
        float s = gTerrain->GetMeltSpeed();
        s = glm::max(0.0f, s * 0.6f);
        gTerrain->SetMeltSpeed(s);
        std::cout << "[Terrain] Melt speed decreased to " << s << "\n";
    }

    // O - toggle projection mode
    static bool useOrtho = false;
    if (curO && !prevO)
    {
        useOrtho = !useOrtho;
        std::cout << "[View] Projection: " << (useOrtho ? "Orthographic" : "Perspective") << "\n";
    }

    // H - toggle on-screen stats (window title)
    if (curH && !prevH)
    {
        gShowStats = !gShowStats;
        std::cout << "[UI] On-screen stats " << (gShowStats ? "enabled" : "disabled") << std::endl;
    }

    // Clouds removed - toggle disabled

    // T - advance time by 1 hour
    if (curT && !prevT && gSkybox)
    {
        gSkybox->AdvanceTime(1.0f);
        std::cout << "[Time] Advance +1 hour -> " << gSkybox->GetTimeOfDay() << "h" << std::endl;
    }

    // Y - toggle day/night
    if (curY && !prevY && gSkybox)
    {
        float t = gSkybox->GetTimeOfDay();
        if (t >= 6.0f && t <= 18.0f)
        {
            gSkybox->SetTimeOfDay(0.0f);
            std::cout << "[Time] Set to night (0:00)" << std::endl;
        }
        else
        {
            gSkybox->SetTimeOfDay(12.0f);
            std::cout << "[Time] Set to day (12:00)" << std::endl;
        }
    }

    // B - toggle auto time progression
    if (curB && !prevB)
    {
        gAutoTime = !gAutoTime;
        std::cout << "[Time] Auto time progression " << (gAutoTime ? "enabled" : "disabled") << std::endl;
    }

    prevP = curP;
    prevR = curR;
    prevBracketL = curBL;
    prevBracketR = curBR;
    prevJ = curJ;
    prevL = curLL;
    prevI = curI;
    prevK = curK;
    prevZ = curZ;
    prevX = curX;
    prevM = curM;
    prevN = curN;
    prevO = curO;
    prevH = curH;
    prevC = curC;
    prevT = curT;
    prevY = curY;
    prevB = curB;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}