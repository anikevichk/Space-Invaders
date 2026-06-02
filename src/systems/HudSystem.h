#pragma once

#include <GL/glew.h>

class HudSystem {
public:
    bool init();
    void draw(int lives, bool fastBulletsActive, float fastBulletsTimeLeft);
    void cleanup();

private:
    GLuint shader = 0;
    GLuint vao = 0;
    GLuint vbo = 0;

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

    void drawHeart(
        float x,
        float y,
        float size
    );
};