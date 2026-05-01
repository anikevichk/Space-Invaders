#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "Mesh.h"

class Player {
public:
    bool init();
    void updateInput(GLFWwindow* window, float deltaTime);
    void draw(const glm::mat4& view, const glm::mat4& projection);
    void cleanup();

    glm::mat4 getModelMatrix() const;

private:
    float x = 0.0f;

    GLuint shipShader = 0;
    GLuint glowShader = 0;
    GLuint shipTexture = 0;

    Mesh shipMesh;
    Mesh glowMesh;
};