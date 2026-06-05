#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <random>
#include <vector>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float life = 0.0f;
    float maxLife = 1.0f;
    float size = 10.0f;
};

class ParticleSystem {
public:
    bool init();
    void update(float deltaTime);
    void draw(const glm::mat4& view, const glm::mat4& projection);
    void cleanup();

    void spawnExplosion(const glm::vec3& position);
    void spawnPlayerHit(const glm::vec3& position);
    void clear();

private:
    struct ParticleVertex {
        glm::vec3 position;
        glm::vec4 color;
        float size;
    };

    GLuint shader = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    bool initialized = false;

    std::vector<Particle> particles;
    std::vector<ParticleVertex> vertices;

    std::mt19937 rng;

    void emitBurst(const glm::vec3& position, int count, const glm::vec4& colorA, const glm::vec4& colorB, float minSpeed, float maxSpeed);
};
