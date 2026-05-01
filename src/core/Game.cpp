#include "Game.h"

#include <iostream>

bool Game::init() {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(
        width,
        height,
        "Space Invaders 3D",
        nullptr,
        nullptr
    );

    if (!window) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetWindowUserPointer(window, this);

    glfwSetFramebufferSizeCallback(
        window,
        [](GLFWwindow* window, int newWidth, int newHeight) {
            Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));

            if (game != nullptr) {
                game->width = newWidth;
                game->height = newHeight;
                game->updateViewport();
            }
        }
    );

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    updateViewport();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    if (!starfield.init()) {
        std::cout << "Failed to initialize starfield\n";
        return false;
    }

    if (!player.init()) {
        std::cout << "Failed to initialize player\n";
        return false;
    }

    if (!bulletSystem.init()) {
        std::cout << "Failed to initialize bullet system\n";
        return false;
    }

    if (!shelterSystem.init()) {
    std::cout << "Failed to initialize shelter system\n";
    return false;
}
    return true;
}

void Game::run() {
    while (!glfwWindowShouldClose(window)) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        player.updateInput(
            window,
            deltaTime,
            camera.getPlayerMinX(),
            camera.getPlayerMaxX()
        );

        glm::mat4 playerModel = player.getModelMatrix();

        bulletSystem.update(window, deltaTime, playerModel, &shelterSystem);

        glClearColor(0.03f, 0.03f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = camera.getProjectionMatrix();

        starfield.draw(view, projection);
        shelterSystem.draw(view, projection);
        player.draw(view, projection);
        bulletSystem.draw(view, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Game::cleanup() {
    shelterSystem.cleanup();
    bulletSystem.cleanup();
    player.cleanup();
    starfield.cleanup();

    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }

    glfwTerminate();
}

void Game::updateViewport() {
    if (height <= 0) {
        height = 1;
    }

    glViewport(0, 0, width, height);
    camera.setSize(static_cast<float>(width), static_cast<float>(height));
}