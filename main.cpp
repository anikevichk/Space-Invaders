#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "Shader.h"
#include "ObjLoader.h"
#include "Texture.h"
#include "Mesh.h"
#include "Input.h"

int main() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        800,
        600,
        "Space Invaders 3D",
        nullptr,
        nullptr
    );

    if (!window) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW\n";
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    GLuint shipShader = createShaderProgram(
        "shaders/ship.vert",
        "shaders/ship.frag"
    );

    GLuint glowShader = createShaderProgram(
        "shaders/engine_glow.vert",
        "shaders/engine_glow.frag"
    );

    if (shipShader == 0 || glowShader == 0) {
        std::cout << "Shader program not created\n";
        return -1;
    }

    std::vector<Vertex> shipVertices = loadObjModel(
        "assets/models/InfraredFurtive/InfraredFurtive.obj",
        "assets/models/InfraredFurtive/"
    );

    if (shipVertices.empty()) {
        std::cout << "Model not loaded\n";
        return -1;
    }

    GLuint shipTexture = loadTexture(
        "assets/models/InfraredFurtive/InfraredFurtive.png"
    );

    if (shipTexture == 0) {
        std::cout << "Texture not loaded\n";
        return -1;
    }

    Mesh shipMesh = createShipMesh(shipVertices);

    // DONT TOUCH!!!
    float engineGlowPoints[] = {
        -0.55f,  0.0f, -2.10f,
        -0.18f,  0.0f, -2.00f,
         0.18f,  0.0f, -2.10f,
         0.55f,  0.0f, -2.00f,
        -3.00f, -0.4f, -1.30f,
         3.00f, -0.4f, -1.30f
    };

    Mesh glowMesh = createPointMesh(
        engineGlowPoints,
        sizeof(engineGlowPoints) / sizeof(float)
    );

    float playerX = 0.0f;
    float lastTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        processInput(window, deltaTime, playerX);

        glClearColor(0.03f, 0.03f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, glm::vec3(playerX, -1.2f, 0.0f));
        model = glm::rotate(
            model,
            glm::radians(190.0f),
            glm::vec3(1.0f, 0.0f, 0.0f)
        );
        model = glm::scale(model, glm::vec3(0.45f));

        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 2.0f, 7.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            800.0f / 600.0f,
            0.1f,
            100.0f
        );

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

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &shipTexture);

    destroyMesh(shipMesh);
    destroyMesh(glowMesh);

    glDeleteProgram(shipShader);
    glDeleteProgram(glowShader);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}