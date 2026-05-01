#include "Player.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "Shader.h"
#include "ObjLoader.h"
#include "Texture.h"
#include "Input.h"

bool Player::init() {
    shipShader = createShaderProgram(
        "shaders/ship.vert",
        "shaders/ship.frag"
    );

    glowShader = createShaderProgram(
        "shaders/engine_glow.vert",
        "shaders/engine_glow.frag"
    );

    if (shipShader == 0 || glowShader == 0) {
        std::cout << "Player shaders not created\n";
        return false;
    }

    std::vector<Vertex> shipVertices = loadObjModel(
        "assets/models/InfraredFurtive/InfraredFurtive.obj",
        "assets/models/InfraredFurtive/"
    );

    if (shipVertices.empty()) {
        std::cout << "Player model not loaded\n";
        return false;
    }

    shipTexture = loadTexture(
        "assets/models/InfraredFurtive/InfraredFurtive.png"
    );

    if (shipTexture == 0) {
        std::cout << "Player texture not loaded\n";
        return false;
    }

    shipMesh = createShipMesh(shipVertices);

    // DONT TOUCH!!!
    float engineGlowPoints[] = {
        -0.55f,  0.0f, -2.10f,
        -0.18f,  0.0f, -2.00f,
         0.18f,  0.0f, -2.10f,
         0.55f,  0.0f, -2.00f,
        -3.00f, -0.4f, -1.30f,
         3.00f, -0.4f, -1.30f
    };

    glowMesh = createPointMesh(
        engineGlowPoints,
        sizeof(engineGlowPoints) / sizeof(float)
    );

    return true;
}

void Player::updateInput(
    GLFWwindow* window,
    float deltaTime,
    float minX,
    float maxX
) {
    processInput(window, deltaTime, x, minX, maxX);
}

glm::mat4 Player::getModelMatrix() const {
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(x, -2.25f, 0.0f));

    model = glm::rotate(
        model,
        glm::radians(190.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
    );

    model = glm::scale(model, glm::vec3(0.30f));

    return model;
}

void Player::draw(const glm::mat4& view, const glm::mat4& projection) {
    glm::mat4 model = getModelMatrix();

    // ---------- SHIP ----------
    glUseProgram(shipShader);

    glUniformMatrix4fv(
        glGetUniformLocation(shipShader, "model"),
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(shipShader, "view"),
        1,
        GL_FALSE,
        glm::value_ptr(view)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(shipShader, "projection"),
        1,
        GL_FALSE,
        glm::value_ptr(projection)
    );

    glUniform3f(
        glGetUniformLocation(shipShader, "lightPos"),
        2.0f,
        5.0f,
        4.0f
    );

    glUniform3f(
        glGetUniformLocation(shipShader, "viewPos"),
        0.0f,
        2.0f,
        7.0f
    );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shipTexture);

    glUniform1i(
        glGetUniformLocation(shipShader, "shipTexture"),
        0
    );

    glBindVertexArray(shipMesh.vao);
    glDrawArrays(GL_TRIANGLES, 0, shipMesh.vertexCount);

    // ---------- ENGINE GLOW ----------
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glUseProgram(glowShader);

    glUniformMatrix4fv(
        glGetUniformLocation(glowShader, "model"),
        1,
        GL_FALSE,
        glm::value_ptr(model)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(glowShader, "view"),
        1,
        GL_FALSE,
        glm::value_ptr(view)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(glowShader, "projection"),
        1,
        GL_FALSE,
        glm::value_ptr(projection)
    );

    glBindVertexArray(glowMesh.vao);
    glDrawArrays(GL_POINTS, 0, glowMesh.vertexCount);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Player::cleanup() {
    if (shipTexture != 0) {
        glDeleteTextures(1, &shipTexture);
        shipTexture = 0;
    }

    destroyMesh(shipMesh);
    destroyMesh(glowMesh);

    if (shipShader != 0) {
        glDeleteProgram(shipShader);
        shipShader = 0;
    }

    if (glowShader != 0) {
        glDeleteProgram(glowShader);
        glowShader = 0;
    }
}