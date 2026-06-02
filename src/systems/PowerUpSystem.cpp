#include "PowerUpSystem.h"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <algorithm>
#include <ctime>

#include "Shader.h"

bool PowerUpSystem::init() {
    rng.seed(static_cast<unsigned int>(time(nullptr)));

    shader = createShaderProgram(
        "shaders/powerup.vert",
        "shaders/powerup.frag"
    );

    if (shader == 0) {
        std::cout << "PowerUp shader not created\n";
        return false;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        100 * sizeof(glm::vec3),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        (void*)0
    );

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    return true;
}

void PowerUpSystem::trySpawn(const glm::vec3& position) {
    std::uniform_int_distribution<int> chance(1, 100);
    int roll = chance(rng);

    PowerUp powerUp;
    powerUp.position = position;
    powerUp.position.y = -1.2f;

    if (roll <= 10) {
        powerUp.type = PowerUpType::FastBullets;
        powerUps.push_back(powerUp);
    }
    else if (roll <= 15) {
        powerUp.type = PowerUpType::ExtraLife;
        powerUps.push_back(powerUp);
    }
}

void PowerUpSystem::update(float deltaTime) {
    for (PowerUp& powerUp : powerUps) {
        powerUp.position.z += fallSpeed * deltaTime;
    }

    powerUps.erase(
        std::remove_if(
            powerUps.begin(),
            powerUps.end(),
            [](const PowerUp& powerUp) {
                return powerUp.position.z > 4.0f;
            }
        ),
        powerUps.end()
    );
}

PowerUpEffect PowerUpSystem::collect(float playerX, float playerZ) {
    for (auto it = powerUps.begin(); it != powerUps.end(); ++it) {
        float shipPickupZ = playerZ + 1.6f;

        bool nearPlayer =
            std::abs(it->position.x - playerX) < 0.55f &&
            std::abs(it->position.z - shipPickupZ) < 0.35f;

        if (nearPlayer) {
            PowerUpEffect effect =
                it->type == PowerUpType::FastBullets
                ? PowerUpEffect::FastBullets
                : PowerUpEffect::ExtraLife;

            powerUps.erase(it);
            return effect;
        }
    }

    return PowerUpEffect::None;
}

void PowerUpSystem::draw(const glm::mat4& view, const glm::mat4& projection) {
    if (powerUps.empty()) {
        return;
    }

    std::vector<glm::vec3> positions;
    positions.reserve(powerUps.size());

    for (const PowerUp& powerUp : powerUps) {
        positions.push_back(powerUp.position);
    }

    glUseProgram(shader);

    glUniformMatrix4fv(
        glGetUniformLocation(shader, "view"),
        1,
        GL_FALSE,
        glm::value_ptr(view)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(shader, "projection"),
        1,
        GL_FALSE,
        glm::value_ptr(projection)
    );

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        positions.size() * sizeof(glm::vec3),
        positions.data()
    );

    glEnable(GL_PROGRAM_POINT_SIZE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glDepthMask(GL_FALSE);

    for (int i = 0; i < static_cast<int>(powerUps.size()); i++) {
        if (powerUps[i].type == PowerUpType::FastBullets) {
            glUniform3f(
                glGetUniformLocation(shader, "powerUpColor"),
                0.2f,
                0.9f,
                1.0f
            );

            glUniform1i(
                glGetUniformLocation(shader, "powerUpShape"),
                0
            );
        }
        else {
            glUniform3f(
                glGetUniformLocation(shader, "powerUpColor"),
                1.0f,
                0.05f,
                0.15f
            );

            glUniform1i(
                glGetUniformLocation(shader, "powerUpShape"),
                1
            );
        }

        glDrawArrays(GL_POINTS, i, 1);
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

void PowerUpSystem::clear() {
    powerUps.clear();
}

void PowerUpSystem::cleanup() {
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if (shader != 0) {
        glDeleteProgram(shader);
        shader = 0;
    }

    powerUps.clear();
}