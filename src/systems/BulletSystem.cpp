#include "BulletSystem.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iostream>

#include "Shader.h"

bool BulletSystem::init() {
    bulletShader = createShaderProgram(
        "shaders/bullet.vert",
        "shaders/bullet.frag"
    );

    if (bulletShader == 0) {
        std::cout << "Bullet shader not created\n";
        return false;
    }

    glGenVertexArrays(1, &bulletVAO);
    glGenBuffers(1, &bulletVBO);

    glBindVertexArray(bulletVAO);

    glBindBuffer(GL_ARRAY_BUFFER, bulletVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        2000 * sizeof(glm::vec3),
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

void BulletSystem::update(
    GLFWwindow* window,
    float deltaTime,
    const glm::mat4& playerModel
) {
    shootCooldown -= deltaTime;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && shootCooldown <= 0.0f) {
        if (bullets.size() < 1000) {
            Bullet bullet;

            glm::vec3 localMuzzle = glm::vec3(0.0f, 0.0f, 2.3f);
            glm::vec3 localDirection = glm::vec3(0.0f, 0.0f, 1.0f);

            bullet.position = glm::vec3(
                playerModel * glm::vec4(localMuzzle, 1.0f)
            );

            glm::vec3 worldDirection = glm::normalize(
                glm::mat3(playerModel) * localDirection
            );

            bullet.direction = worldDirection;
            bullet.velocity = worldDirection * 7.0f;

            bullets.push_back(bullet);
        }

        shootCooldown = 0.20f;
    }

    for (Bullet& bullet : bullets) {
        bullet.position += bullet.velocity * deltaTime;
    }

    bullets.erase(
        std::remove_if(
            bullets.begin(),
            bullets.end(),
            [](const Bullet& bullet) {
                return bullet.position.x < -10.0f ||
                       bullet.position.x >  10.0f ||
                       bullet.position.y < -10.0f ||
                       bullet.position.y >  10.0f ||
                       bullet.position.z < -15.0f ||
                       bullet.position.z >  15.0f;
            }
        ),
        bullets.end()
    );
}

void BulletSystem::draw(const glm::mat4& view, const glm::mat4& projection) {
    std::vector<glm::vec3> bulletLinePoints;
    bulletLinePoints.reserve(bullets.size() * 2);

    for (const Bullet& bullet : bullets) {
        glm::vec3 start = bullet.position;
        glm::vec3 end = bullet.position + bullet.direction * 0.45f;

        bulletLinePoints.push_back(start);
        bulletLinePoints.push_back(end);
    }

    if (bulletLinePoints.empty()) {
        return;
    }

    glUseProgram(bulletShader);

    glUniformMatrix4fv(
        glGetUniformLocation(bulletShader, "view"),
        1,
        GL_FALSE,
        glm::value_ptr(view)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(bulletShader, "projection"),
        1,
        GL_FALSE,
        glm::value_ptr(projection)
    );

    glBindVertexArray(bulletVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bulletVBO);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        bulletLinePoints.size() * sizeof(glm::vec3),
        bulletLinePoints.data()
    );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    // glow
    glUniform3f(
        glGetUniformLocation(bulletShader, "bulletColor"),
        0.2f,
        0.9f,
        1.0f
    );

    glUniform1f(
        glGetUniformLocation(bulletShader, "alpha"),
        0.35f
    );

    glLineWidth(12.0f);

    glDrawArrays(
        GL_LINES,
        0,
        static_cast<GLsizei>(bulletLinePoints.size())
    );

    // core
    glUniform3f(
        glGetUniformLocation(bulletShader, "bulletColor"),
        0.75f,
        1.0f,
        1.0f
    );

    glUniform1f(
        glGetUniformLocation(bulletShader, "alpha"),
        1.0f
    );

    glLineWidth(3.0f);

    glDrawArrays(
        GL_LINES,
        0,
        static_cast<GLsizei>(bulletLinePoints.size())
    );

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void BulletSystem::cleanup() {
    if (bulletVAO != 0) {
        glDeleteVertexArrays(1, &bulletVAO);
        bulletVAO = 0;
    }

    if (bulletVBO != 0) {
        glDeleteBuffers(1, &bulletVBO);
        bulletVBO = 0;
    }

    if (bulletShader != 0) {
        glDeleteProgram(bulletShader);
        bulletShader = 0;
    }

    bullets.clear();
}