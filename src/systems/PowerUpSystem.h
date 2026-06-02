#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <random>

enum class PowerUpType {
    FastBullets,
    ExtraLife
};

enum class PowerUpEffect {
    None,
    FastBullets,
    ExtraLife
};

struct PowerUp {
    glm::vec3 position;
    PowerUpType type;
};

class PowerUpSystem {
public:
    bool init();
    void update(float deltaTime);
    void draw(const glm::mat4& view, const glm::mat4& projection);
    void cleanup();

    void trySpawn(const glm::vec3& position);
    PowerUpEffect collect(float playerX, float playerZ);
    void clear();

private:
    std::vector<PowerUp> powerUps;

    GLuint shader = 0;
    GLuint vao = 0;
    GLuint vbo = 0;

    std::mt19937 rng;

    float fallSpeed = 1.8f;
};