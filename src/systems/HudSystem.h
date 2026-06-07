#pragma once

#include <GL/glew.h>
#include <string>

class HudSystem {
public:
    bool init();
    void setScreenSize(int width, int height);
    void draw(int lives, bool fastBulletsActive, float fastBulletsTimeLeft, int score);
    void drawStartScreen();
    void drawGameOverScreen(int score);
    void drawWinScreen(int score);
    void cleanup();

private:
    GLuint shader = 0;
    GLuint vao = 0;
    GLuint vbo = 0;

    int screenWidth = 800;
    int screenHeight = 600;

    void drawRect(
        float x,
        float y,
        float w,
        float h,
        float r,
        float g,
        float b,
        float a
    );

    void drawRectPx(
        float x,
        float y,
        float w,
        float h,
        float r,
        float g,
        float b,
        float a
    );

    void drawHeart(
        float x,
        float y,
        float size
    );

    void drawHeartPx(
        float x,
        float y,
        float size
    );

    void drawText(
        const std::string& text,
        float x,
        float y,
        float scale,
        float r,
        float g,
        float b,
        float a
    );

    void drawTextPx(
        const std::string& text,
        float x,
        float y,
        float pixelSize,
        float r,
        float g,
        float b,
        float a
    );

    void drawCenteredText(
        const std::string& text,
        float centerX,
        float y,
        float scale,
        float r,
        float g,
        float b,
        float a
    );

    void drawCenteredTextPx(
        const std::string& text,
        float centerX,
        float y,
        float pixelSize,
        float r,
        float g,
        float b,
        float a
    );

    void drawOverlay(
        const std::string& title,
        float titleR,
        float titleG,
        float titleB,
        const std::string& subtitle,
        int score
    );
};
