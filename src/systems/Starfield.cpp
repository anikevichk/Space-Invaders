#include "Starfield.h"
#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <random>
#include <iostream>

bool Starfield::init() {
    shader = createShaderProgram(
        "shaders/star.vert",
        "shaders/star.frag"
    );

    if (shader == 0) {
        std::cout << "Star shader not created\n";
        return false;
    }

    std::vector<glm::vec3> stars;
    stars.reserve(starCount);

    std::mt19937 rng(static_cast<unsigned int>(time(nullptr)));

    std::uniform_real_distribution<float> xDist(-8.0f, 8.0f);
    std::uniform_real_distribution<float> yDist(-4.0f, 5.0f);
    std::uniform_real_distribution<float> zDist(-12.0f, -3.0f);

    for (int i = 0; i < starCount; i++) {
        stars.emplace_back(
            xDist(rng),
            yDist(rng),
            zDist(rng)
        );
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        stars.size() * sizeof(glm::vec3),
        stars.data(),
        GL_STATIC_DRAW
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

void Starfield::draw(const glm::mat4& view, const glm::mat4& projection) {
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
    glDrawArrays(GL_POINTS, 0, starCount);
}

void Starfield::cleanup() {
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
}