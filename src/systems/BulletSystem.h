#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <vector>
class ShelterSystem;
class EnemySystem;

struct Bullet {
    glm::vec3 position;
    glm::vec3 prevPosition;
    glm::vec3 velocity;
    glm::vec3 direction;
    bool      markedForRemoval = false;
};

class BulletSystem {
public:
    bool init();
    void update(
        GLFWwindow* window,
        float deltaTime,
        const glm::mat4& playerModel,
        ShelterSystem* shelterSystem,
        EnemySystem* enemySystem = nullptr
    );
    void draw(const glm::mat4& view, const glm::mat4& projection);
    void cleanup();

private:
    GLuint bulletShader = 0;
    GLuint bulletVAO = 0;
    GLuint bulletVBO = 0;

    std::vector<Bullet> bullets;

    float shootCooldown = 0.0f;
};