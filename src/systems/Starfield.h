#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class Starfield {
public:
    bool init();
    void update(float deltaTime);
    void draw(const glm::mat4& view, const glm::mat4& projection, bool drawComets = false);
    void cleanup();

private:
    struct Comet {
        glm::vec3 head;
        glm::vec3 velocity;
        float tailLength = 1.6f;
    };

    GLuint starShader = 0;
    GLuint cometShader = 0;

    GLuint starVao = 0;
    GLuint starVbo = 0;

    GLuint cometVao = 0;
    GLuint cometVbo = 0;

    int starCount = 300;
    float time = 0.0f;

    std::vector<glm::vec3> stars;
    std::vector<float> starSpeeds;
    std::vector<Comet> comets;

    void respawnStar(size_t index);
    void respawnComet(size_t index, bool randomizePosition);
};
