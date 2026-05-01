#include "Input.h"

void processInput(
    GLFWwindow* window,
    float deltaTime,
    float& playerX,
    float minX,
    float maxX
) {
    const float speed = 4.2f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        playerX -= speed * deltaTime;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        playerX += speed * deltaTime;
    }

    if (playerX < minX) {
        playerX = minX;
    }

    if (playerX > maxX) {
        playerX = maxX;
    }
}