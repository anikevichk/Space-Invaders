#include "Game.h"

#include <iostream>
#include <glm/glm.hpp>

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

    if (!particleSystem.init()) {
        std::cout << "Failed to initialize particle system\n";
        return false;
    }

    resetGame();
    state = State::StartMenu;
    lastTime = static_cast<float>(glfwGetTime());
    return true;
}

bool Game::pressedOnce(int key, bool& wasDown) {
    bool isDown = glfwGetKey(window, key) == GLFW_PRESS;
    bool pressed = isDown && !wasDown;
    wasDown = isDown;
    return pressed;
}

void Game::resetGame() {
    lives = 3;
    stateTimer = 0.0f;
    playerInvulnerabilityTimer = 0.0f;

    player.reset();
    enemySystem.reset();
    shelterSystem.reset();
    bulletSystem.clear();
    powerUpSystem.clear();
    particleSystem.clear();
}

void Game::updatePlaying(float deltaTime) {
    if (playerInvulnerabilityTimer > 0.0f) {
        playerInvulnerabilityTimer -= deltaTime;

        if (playerInvulnerabilityTimer < 0.0f) {
            playerInvulnerabilityTimer = 0.0f;
        }
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
        &powerUpSystem,
        &particleSystem
    );

    powerUpSystem.update(deltaTime);
    particleSystem.update(deltaTime);

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

    if (playerInvulnerabilityTimer <= 0.0f && enemySystem.playerHit(player.getX(), 0.0f)) {
        lives--;

        std::cout << "Player hit, lives = " << lives << "\n";

        if (lives <= 0) {
            particleSystem.spawnExplosion(glm::vec3(player.getX(), -1.65f, 0.0f));
            state = State::PlayerDying;
            stateTimer = 1.35f;
            bulletSystem.clear();
            powerUpSystem.clear();
        } else {
            particleSystem.spawnPlayerHit(glm::vec3(player.getX(), -1.65f, 0.0f));
            playerInvulnerabilityTimer = 0.75f;
        }
    }

    if (state == State::Playing && enemySystem.allDead()) {
        state = State::Won;
        bulletSystem.clear();
        powerUpSystem.clear();
        std::cout << "You won\n";
    }
}

void Game::drawScene(bool drawPlayerShip) {
    glClearColor(0.03f, 0.03f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view       = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix();

    starfield.draw(view, projection, false);
    shelterSystem.draw(view, projection);
    enemySystem.draw(view, projection);
    powerUpSystem.draw(view, projection);

    if (drawPlayerShip) {
        player.draw(view, projection);
    }

    bulletSystem.draw(view, projection);
    particleSystem.draw(view, projection);
}

void Game::drawMenuScene() {
    glClearColor(0.02f, 0.02f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view       = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix();

    starfield.draw(view, projection, true);
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

        starfield.update(deltaTime);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        bool enterPressed = pressedOnce(GLFW_KEY_ENTER, enterWasDown) ||
                            pressedOnce(GLFW_KEY_KP_ENTER, kpEnterWasDown);
        bool rPressed = pressedOnce(GLFW_KEY_R, rWasDown);

        if (state == State::StartMenu) {
            if (enterPressed) {
                resetGame();
                state = State::Playing;
            }

            drawMenuScene();
            hudSystem.drawStartScreen();
        } else if (state == State::Playing) {
            updatePlaying(deltaTime);
            drawScene(true);
            hudSystem.draw(
                lives,
                bulletSystem.isFastBulletsActive(),
                bulletSystem.getFastBulletsTimeLeft(),
                enemySystem.getScore()
            );
        } else if (state == State::PlayerDying) {
            stateTimer -= deltaTime;
            particleSystem.update(deltaTime);

            if (stateTimer <= 0.0f) {
                state = State::GameOver;
                stateTimer = 0.0f;
            }

            drawScene(false);
            hudSystem.draw(
                0,
                false,
                0.0f,
                enemySystem.getScore()
            );
        } else if (state == State::GameOver) {
            if (rPressed) {
                resetGame();
                state = State::Playing;
            }

            particleSystem.update(deltaTime);
            drawScene(false);
            hudSystem.drawGameOverScreen(enemySystem.getScore());
        } else if (state == State::Won) {
            if (rPressed) {
                resetGame();
                state = State::Playing;
            }

            particleSystem.update(deltaTime);
            drawScene(true);
            hudSystem.drawWinScreen(enemySystem.getScore());
        }

        glfwSwapBuffers(window);
    }
}

void Game::cleanup() {
    particleSystem.cleanup();
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
    hudSystem.setScreenSize(width, height);
}
