#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

class Starfield {
public:
    bool init();
    void draw(const glm::mat4& view, const glm::mat4& projection);
    void cleanup();

private:
    GLuint shader = 0;
    GLuint vao = 0;
    GLuint vbo = 0;
    int starCount = 300;
};