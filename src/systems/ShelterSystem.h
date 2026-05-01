#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class ShelterSystem {
public:
    bool init();
    void draw(const glm::mat4& view, const glm::mat4& projection);
    void cleanup();

    bool hitByBullet(const glm::vec3& bulletStart, const glm::vec3& bulletEnd);

private:
    struct Block {
        glm::vec3 position;
        glm::vec3 scale;
    };

    GLuint shader = 0;
    GLuint vao = 0;
    GLuint vbo = 0;

    std::vector<Block> blocks;

    void createShelter(float centerX);
    bool bulletHitsBlock(
    const glm::vec3& bulletPosition,
    const Block& block
    ) const;
};