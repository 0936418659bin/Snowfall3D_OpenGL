#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float size;
    float life;
    float rotation;
    float rotationSpeed;
    float weight; // Trọng lượng ảnh hưởng tốc độ rơi
    
    Particle() 
        : position(0.0f), velocity(0.0f), color(1.0f), 
          size(1.0f), life(0.0f), rotation(0.0f), 
          rotationSpeed(0.0f), weight(1.0f) {}
};

#endif