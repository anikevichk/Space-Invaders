#include "Starfield.h"
#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <ctime>
#include <iostream>
#include <random>
#include <vector>

namespace {
    std::mt19937& globalRng() {
        static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));
        return rng;
    }

    float randomRange(float minValue, float maxValue) {
        std::uniform_real_distribution<float> dist(minValue, maxValue);
        return dist(globalRng());
    }
}

bool Starfield::init() {
    starShader = createShaderProgram(
        "shaders/star.vert",
        "shaders/star.frag"
    );

    cometShader = createShaderProgram(
        "shaders/comet.vert",
        "shaders/comet.frag"
    );

    if (starShader == 0 || cometShader == 0) {
        std::cout << "Starfield shaders not created\n";
        return false;
    }

    stars.resize(static_cast<size_t>(starCount));
    starSpeeds.resize(static_cast<size_t>(starCount));

    for (size_t i = 0; i < stars.size(); ++i) {
        respawnStar(i);
        stars[i].z = randomRange(-14.0f, -2.5f);
    }

    comets.resize(3);
    for (size_t i = 0; i < comets.size(); ++i) {
        respawnComet(i, true);
        comets[i].head.x = randomRange(-9.0f, 8.0f);
        comets[i].head.y = randomRange(-2.0f, 5.5f);
        comets[i].head.z = randomRange(-10.0f, -4.5f);
    }

    glGenVertexArrays(1, &starVao);
    glGenBuffers(1, &starVbo);

    glBindVertexArray(starVao);
    glBindBuffer(GL_ARRAY_BUFFER, starVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(stars.size() * sizeof(glm::vec3)),
        stars.data(),
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

    glGenVertexArrays(1, &cometVao);
    glGenBuffers(1, &cometVbo);

    glBindVertexArray(cometVao);
    glBindBuffer(GL_ARRAY_BUFFER, cometVbo);
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(comets.size() * 2 * 4 * sizeof(float)),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        1,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return true;
}

void Starfield::respawnStar(size_t index) {
    stars[index] = glm::vec3(
        randomRange(-8.5f, 8.5f),
        randomRange(-4.8f, 5.5f),
        -14.0f
    );

    starSpeeds[index] = randomRange(2.0f, 6.5f);
}

void Starfield::respawnComet(size_t index, bool randomizePosition) {
    Comet& comet = comets[index];

    int direction = static_cast<int>(randomRange(0.0f, 4.0f));

    if (direction == 0) {
        // right -> left
        comet.head = glm::vec3(
            randomRange(8.5f, 10.8f),
            randomRange(-1.5f, 5.8f),
            randomRange(-9.5f, -4.0f)
        );
        comet.velocity = glm::vec3(
            randomRange(-6.5f, -4.5f),
            randomRange(-1.0f, 1.0f),
            randomRange(0.2f, 0.8f)
        );
    } else if (direction == 1) {
        // left -> right
        comet.head = glm::vec3(
            randomRange(-10.8f, -8.5f),
            randomRange(-1.5f, 5.8f),
            randomRange(-9.5f, -4.0f)
        );
        comet.velocity = glm::vec3(
            randomRange(4.5f, 6.5f),
            randomRange(-1.0f, 1.0f),
            randomRange(0.2f, 0.8f)
        );
    } else if (direction == 2) {
        // top -> lower part of screen
        comet.head = glm::vec3(
            randomRange(-7.5f, 7.5f),
            randomRange(5.8f, 7.2f),
            randomRange(-9.5f, -4.0f)
        );
        comet.velocity = glm::vec3(
            randomRange(-1.5f, 1.5f),
            randomRange(-4.6f, -3.0f),
            randomRange(0.2f, 0.8f)
        );
    } else {
        // diagonal, but not always from the same corner
        bool fromRight = randomRange(0.0f, 1.0f) > 0.5f;
        comet.head = glm::vec3(
            fromRight ? randomRange(8.5f, 10.8f) : randomRange(-10.8f, -8.5f),
            randomRange(4.8f, 7.0f),
            randomRange(-9.5f, -4.0f)
        );
        comet.velocity = glm::vec3(
            fromRight ? randomRange(-5.8f, -4.0f) : randomRange(4.0f, 5.8f),
            randomRange(-3.4f, -2.0f),
            randomRange(0.2f, 0.8f)
        );
    }

    if (randomizePosition) {
        comet.head += comet.velocity * randomRange(0.0f, 2.0f);
    }

    comet.tailLength = randomRange(1.3f, 2.2f);
}

void Starfield::update(float deltaTime) {
    time += deltaTime;

    for (size_t i = 0; i < stars.size(); ++i) {
        stars[i].z += starSpeeds[i] * deltaTime;

        if (stars[i].z > -2.5f) {
            respawnStar(i);
        }
    }

    for (size_t i = 0; i < comets.size(); ++i) {
        comets[i].head += comets[i].velocity * deltaTime;

        if (comets[i].head.x < -11.0f || comets[i].head.x > 11.0f || comets[i].head.y < -6.0f || comets[i].head.y > 7.5f || comets[i].head.z > -2.0f) {
            respawnComet(i, false);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, starVbo);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        static_cast<GLsizeiptr>(stars.size() * sizeof(glm::vec3)),
        stars.data()
    );

    std::vector<float> cometVertices;
    cometVertices.reserve(comets.size() * 8);

    for (const Comet& comet : comets) {
        glm::vec3 direction = glm::normalize(comet.velocity);
        glm::vec3 tail = comet.head - direction * comet.tailLength;

        cometVertices.push_back(comet.head.x);
        cometVertices.push_back(comet.head.y);
        cometVertices.push_back(comet.head.z);
        cometVertices.push_back(1.0f);

        cometVertices.push_back(tail.x);
        cometVertices.push_back(tail.y);
        cometVertices.push_back(tail.z);
        cometVertices.push_back(0.0f);
    }

    glBindBuffer(GL_ARRAY_BUFFER, cometVbo);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        static_cast<GLsizeiptr>(cometVertices.size() * sizeof(float)),
        cometVertices.data()
    );
}

void Starfield::draw(const glm::mat4& view, const glm::mat4& projection, bool drawComets) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glUseProgram(starShader);
    glUniformMatrix4fv(
        glGetUniformLocation(starShader, "view"),
        1,
        GL_FALSE,
        glm::value_ptr(view)
    );
    glUniformMatrix4fv(
        glGetUniformLocation(starShader, "projection"),
        1,
        GL_FALSE,
        glm::value_ptr(projection)
    );
    glUniform1f(
        glGetUniformLocation(starShader, "uTime"),
        time
    );

    glBindVertexArray(starVao);
    glDrawArrays(GL_POINTS, 0, starCount);

    if (drawComets) {
        glUseProgram(cometShader);
        glUniformMatrix4fv(
            glGetUniformLocation(cometShader, "view"),
            1,
            GL_FALSE,
            glm::value_ptr(view)
        );
        glUniformMatrix4fv(
            glGetUniformLocation(cometShader, "projection"),
            1,
            GL_FALSE,
            glm::value_ptr(projection)
        );

        glLineWidth(2.0f);
        glBindVertexArray(cometVao);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(comets.size() * 2));
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

void Starfield::cleanup() {
    if (starVao != 0) {
        glDeleteVertexArrays(1, &starVao);
        starVao = 0;
    }

    if (starVbo != 0) {
        glDeleteBuffers(1, &starVbo);
        starVbo = 0;
    }

    if (cometVao != 0) {
        glDeleteVertexArrays(1, &cometVao);
        cometVao = 0;
    }

    if (cometVbo != 0) {
        glDeleteBuffers(1, &cometVbo);
        cometVbo = 0;
    }

    if (starShader != 0) {
        glDeleteProgram(starShader);
        starShader = 0;
    }

    if (cometShader != 0) {
        glDeleteProgram(cometShader);
        cometShader = 0;
    }
}
