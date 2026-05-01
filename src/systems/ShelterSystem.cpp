#include "ShelterSystem.h"
#include "Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iostream>

bool ShelterSystem::init() {
    shader = createShaderProgram(
        "shaders/shelter.vert",
        "shaders/shelter.frag"
    );

    if (shader == 0) {
        std::cout << "Shelter shader not created\n";
        return false;
    }

    float cubeVertices[] = {
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(cubeVertices),
        cubeVertices,
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    createShelter(-2.7f);
    createShelter(-0.9f);
    createShelter(0.9f);
    createShelter(2.7f);

    return true;
}

void ShelterSystem::createShelter(float centerX) {
    const float y = -1.45f;
    const float z = -2.1f;

    const glm::vec3 blockScale(0.18f, 0.14f, 0.24f);

    blocks.push_back({ glm::vec3(centerX - 0.45f, y, z), blockScale });
    blocks.push_back({ glm::vec3(centerX - 0.27f, y, z), blockScale });
    blocks.push_back({ glm::vec3(centerX + 0.27f, y, z), blockScale });
    blocks.push_back({ glm::vec3(centerX + 0.45f, y, z), blockScale });

    blocks.push_back({ glm::vec3(centerX - 0.54f, y + 0.14f, z), blockScale });
    blocks.push_back({ glm::vec3(centerX - 0.36f, y + 0.14f, z), blockScale });
    blocks.push_back({ glm::vec3(centerX - 0.18f, y + 0.14f, z), blockScale });
    blocks.push_back({ glm::vec3(centerX,         y + 0.14f, z), blockScale });
    blocks.push_back({ glm::vec3(centerX + 0.18f, y + 0.14f, z), blockScale });
    blocks.push_back({ glm::vec3(centerX + 0.36f, y + 0.14f, z), blockScale });
    blocks.push_back({ glm::vec3(centerX + 0.54f, y + 0.14f, z), blockScale });

    blocks.push_back({ glm::vec3(centerX - 0.36f, y + 0.28f, z), blockScale });
    blocks.push_back({ glm::vec3(centerX - 0.18f, y + 0.28f, z), blockScale });
    blocks.push_back({ glm::vec3(centerX,         y + 0.28f, z), blockScale });
    blocks.push_back({ glm::vec3(centerX + 0.18f, y + 0.28f, z), blockScale });
    blocks.push_back({ glm::vec3(centerX + 0.36f, y + 0.28f, z), blockScale });
}

bool ShelterSystem::bulletHitsBlock(
    const glm::vec3& bulletPosition,
    const Block& block
) const {
    float minX = block.position.x - block.scale.x * 0.5f - 0.08f;
    float maxX = block.position.x + block.scale.x * 0.5f + 0.08f;

    float minZ = block.position.z - block.scale.z * 0.5f - 0.18f;
    float maxZ = block.position.z + block.scale.z * 0.5f + 0.18f;

    return bulletPosition.x >= minX &&
           bulletPosition.x <= maxX &&
           bulletPosition.z >= minZ &&
           bulletPosition.z <= maxZ;
}

bool ShelterSystem::hitByBullet(
    const glm::vec3& bulletStart,
    const glm::vec3& bulletEnd
) {
    const int steps = 8;

    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        for (int i = 0; i <= steps; i++) {
            float t = static_cast<float>(i) / static_cast<float>(steps);
            glm::vec3 point = bulletStart + (bulletEnd - bulletStart) * t;

            if (bulletHitsBlock(point, *it)) {
                blocks.erase(it);
                return true;
            }
        }
    }

    return false;
}

void ShelterSystem::draw(const glm::mat4& view, const glm::mat4& projection) {
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

    for (const Block& block : blocks) {
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, block.position);
        model = glm::scale(model, block.scale);

        glUniformMatrix4fv(
            glGetUniformLocation(shader, "model"),
            1,
            GL_FALSE,
            glm::value_ptr(model)
        );

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glBindVertexArray(0);
}

void ShelterSystem::cleanup() {
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

    blocks.clear();
}