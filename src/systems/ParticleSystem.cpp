#include "ParticleSystem.h"
#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <iostream>
#include <random>

static float randomFloat(std::mt19937& rng, float minValue, float maxValue)
{
    std::uniform_real_distribution<float> distribution(minValue, maxValue);
    return distribution(rng);
}

bool ParticleSystem::init() {
    rng.seed(static_cast<unsigned>(time(nullptr)));

    shader = createShaderProgram(
        "shaders/particle.vert",
        "shaders/particle.frag"
    );

    if (shader == 0) {
        std::cout << "Particle shader not created - particles disabled\n";
        return false;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        2000 * sizeof(ParticleVertex),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(ParticleVertex),
        (void*)offsetof(ParticleVertex, position)
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(ParticleVertex),
        (void*)offsetof(ParticleVertex, color)
    );
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(
        2,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(ParticleVertex),
        (void*)offsetof(ParticleVertex, size)
    );
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    initialized = true;
    return true;
}

void ParticleSystem::emitBurst(
    const glm::vec3& position,
    int count,
    const glm::vec4& colorA,
    const glm::vec4& colorB,
    float minSpeed,
    float maxSpeed
) {
    if (!initialized) {
        return;
    }

    std::uniform_real_distribution<float> angleDist(0.0f, 6.2831853f);
    std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);
    std::uniform_real_distribution<float> upDist(-0.35f, 0.75f);
    std::uniform_real_distribution<float> lifeDist(0.25f, 0.55f);
    std::uniform_real_distribution<float> sizeDist(4.0f, 9.0f);
    std::uniform_real_distribution<float> mixDist(0.0f, 1.0f);

    for (int i = 0; i < count && particles.size() < 1800; i++) {
        float angle = angleDist(rng);
        float speed = speedDist(rng);
        float mix = mixDist(rng);

        Particle p;
        p.position = position;
        p.velocity = glm::vec3(
            std::cos(angle) * speed,
            upDist(rng),
            std::sin(angle) * speed
        );
        p.color = colorA * (1.0f - mix) + colorB * mix;
        p.life = lifeDist(rng);
        p.maxLife = p.life;
        p.size = sizeDist(rng);

        particles.push_back(p);
    }
}

void ParticleSystem::spawnExplosion(const glm::vec3& position)
{
    if (!initialized) {
        return;
    }

    const int count = 45;

    for (int i = 0; i < count && particles.size() < 1800; i++) {
        Particle p;

        p.position = position;

        float angle = randomFloat(rng, 0.0f, 6.28318f);
        float speed = randomFloat(rng, 1.5f, 5.5f);

        p.velocity = glm::vec3(
            std::cos(angle) * speed,
            randomFloat(rng, -0.8f, 2.2f),
            std::sin(angle) * speed
        );

        p.life = randomFloat(rng, 0.25f, 0.55f);
        p.maxLife = p.life;

        p.size = randomFloat(rng, 4.0f, 9.0f);

        float type = randomFloat(rng, 0.0f, 1.0f);

        if (type < 0.55f) {
            p.color = glm::vec4(1.0f, 0.75f, 0.15f, 1.0f);
        } else if (type < 0.85f) {
            p.color = glm::vec4(1.0f, 0.25f, 0.05f, 1.0f);
        } else {
            p.color = glm::vec4(0.75f, 0.75f, 0.75f, 0.8f);
        }

        particles.push_back(p);
    }
}

void ParticleSystem::spawnPlayerHit(const glm::vec3& position)
{
    if (!initialized) {
        return;
    }

    const int count = 25;

    for (int i = 0; i < count && particles.size() < 1800; i++) {
        Particle p;

        p.position = position;

        float angle = randomFloat(rng, 0.0f, 6.28318f);
        float speed = randomFloat(rng, 1.0f, 3.5f);

        p.velocity = glm::vec3(
            std::cos(angle) * speed,
            randomFloat(rng, 0.0f, 2.0f),
            std::sin(angle) * speed
        );

        p.life = randomFloat(rng, 0.18f, 0.35f);
        p.maxLife = p.life;

        p.size = randomFloat(rng, 3.0f, 7.0f);
        p.color = glm::vec4(1.0f, 0.05f, 0.15f, 1.0f);

        particles.push_back(p);
    }
}

void ParticleSystem::update(float deltaTime) {
    if (!initialized) {
        return;
    }

    for (Particle& p : particles) {
        p.life -= deltaTime;
        p.velocity *= 0.94f;
        p.position += p.velocity * deltaTime;
    }

    particles.erase(
        std::remove_if(
            particles.begin(),
            particles.end(),
            [](const Particle& p) {
                return p.life <= 0.0f;
            }
        ),
        particles.end()
    );
}

void ParticleSystem::draw(const glm::mat4& view, const glm::mat4& projection) {
    if (!initialized || shader == 0 || vao == 0 || vbo == 0 || particles.empty()) {
        return;
    }

    vertices.clear();
    vertices.reserve(particles.size());

    for (const Particle& p : particles) {
        float ratio = p.life / p.maxLife;

        ParticleVertex v;
        v.position = p.position;
        v.color = p.color;
        v.color.a *= ratio;
        v.size = p.size * (0.45f + ratio * 0.55f);

        vertices.push_back(v);
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
        vertices.size() * sizeof(ParticleVertex),
        vertices.data()
    );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(vertices.size()));

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void ParticleSystem::clear() {
    particles.clear();
}

void ParticleSystem::cleanup() {
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

    initialized = false;
    particles.clear();
    vertices.clear();
}