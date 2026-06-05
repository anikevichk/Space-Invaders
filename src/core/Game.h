#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Player.h"
#include "BulletSystem.h"
#include "Camera.h"
#include "Starfield.h"
#include "ShelterSystem.h"
#include "EnemySystem.h"
#include "PowerUpSystem.h"
#include "HudSystem.h"
#include "ParticleSystem.h"

class Game {
public:
    bool init();
    void run();
    void cleanup();

private:
    enum class State {
        StartMenu,
        Playing,
        PlayerDying,
        GameOver,
        Won
    };

    GLFWwindow* window = nullptr;

    int width = 800;
    int height = 600;

    Player player;
    BulletSystem bulletSystem;
    Starfield starfield;
    ShelterSystem shelterSystem;
    EnemySystem enemySystem;
    Camera camera = Camera(800.0f, 600.0f);

    float lastTime = 0.0f;
    void updateViewport();

    PowerUpSystem powerUpSystem;

    int lives = 3;
    const int maxLives = 5;

    HudSystem hudSystem;
    ParticleSystem particleSystem;

    State state = State::StartMenu;
    float stateTimer = 0.0f;
    float playerInvulnerabilityTimer = 0.0f;

    bool enterWasDown = false;
    bool kpEnterWasDown = false;
    bool rWasDown = false;

    void resetGame();
    bool pressedOnce(int key, bool& wasDown);
    void updatePlaying(float deltaTime);
    void drawScene(bool drawPlayerShip);
    void drawMenuScene();
};
