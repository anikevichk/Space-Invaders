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

    if (!enemySystem.init()) {
        std::cout << "Failed to initialize enemy system\n";
        return false;
    }

    if (!powerUpSystem.init()) {
        std::cout << "Failed to initialize power-up system\n";
        return false;
    }

    if (!hudSystem.init()) {
        std::cout << "Failed to initialize HUD system\n";
        return false;
    }

    lastTime = static_cast<float>(glfwGetTime());
    return true;
}

void Game::run() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        if (deltaTime > 0.033f) {
            deltaTime = 0.033f;
        }

        player.updateInput(
            window,
            deltaTime,
            camera.getPlayerMinX(),
            camera.getPlayerMaxX()
        );

        glm::mat4 playerModel = player.getModelMatrix();

        enemySystem.update(deltaTime, camera.getPlayerMinX(), camera.getPlayerMaxX(), &shelterSystem);
        bulletSystem.update(
            window,
            deltaTime,
            playerModel,
            &shelterSystem,
            &enemySystem,
            &powerUpSystem
        );

        powerUpSystem.update(deltaTime);

        PowerUpEffect effect = powerUpSystem.collect(player.getX(), 0.0f);

        if (effect == PowerUpEffect::FastBullets) {
            bulletSystem.activateFastBullets(8.0f);
            std::cout << "Power-up: fast bullets\n";
        }

        if (effect == PowerUpEffect::ExtraLife) {
            if (lives < maxLives) {
                lives++;
            }

            std::cout << "Power-up: extra life, lives = " << lives << "\n";
        }

        if (enemySystem.playerHit(player.getX(), 0.0f)) {
            lives--;

            std::cout << "Player hit, lives = " << lives << "\n";

            if (lives <= 0) {
                lives = 3;
                enemySystem.reset();
                powerUpSystem.clear();
            }
        }

        if (enemySystem.allDead()) {
            enemySystem.reset();
        }

        glClearColor(0.03f, 0.03f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view       = camera.getViewMatrix();
        glm::mat4 projection = camera.getProjectionMatrix();

        starfield.draw(view, projection);
        shelterSystem.draw(view, projection);
        enemySystem.draw(view, projection);
        powerUpSystem.draw(view, projection);
        player.draw(view, projection);
        bulletSystem.draw(view, projection);

        hudSystem.draw(
            lives,
            bulletSystem.isFastBulletsActive(),
            bulletSystem.getFastBulletsTimeLeft()
        );

        glfwSwapBuffers(window);
    }
}

void Game::cleanup() {
    hudSystem.cleanup();
    powerUpSystem.cleanup();
    enemySystem.cleanup();
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