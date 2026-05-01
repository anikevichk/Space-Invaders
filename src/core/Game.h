#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Player.h"
#include "BulletSystem.h"
#include "Camera.h"
#include "Starfield.h"

class Game {
public:
    bool init();
    void run();
    void cleanup();

private:
    GLFWwindow* window = nullptr;

    const int width = 800;
    const int height = 600;

    Player player;
    BulletSystem bulletSystem;
    Starfield starfield;
    Camera camera = Camera(800.0f, 600.0f);

    float lastTime = 0.0f;
};