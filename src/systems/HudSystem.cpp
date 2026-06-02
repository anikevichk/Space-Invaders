#include "HudSystem.h"

#include <iostream>
#include "Shader.h"

bool HudSystem::init() {
    shader = createShaderProgram(
        "shaders/hud.vert",
        "shaders/hud.frag"
    );

    if (shader == 0) {
        std::cout << "HUD shader not created\n";
        return false;
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        24 * sizeof(float),
        nullptr,
        GL_DYNAMIC_DRAW
    );

    // position: vec2
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    // uv: vec2
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float),
        (void*)(2 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return true;
}

void HudSystem::drawRect(
    float x,
    float y,
    float w,
    float h,
    float r,
    float g,
    float b,
    float a
) {
    float vertices[] = {
        // pos x, pos y, uv x, uv y
        x,     y,     0.0f, 1.0f,
        x + w, y,     1.0f, 1.0f,
        x + w, y - h, 1.0f, 0.0f,

        x,     y,     0.0f, 1.0f,
        x + w, y - h, 1.0f, 0.0f,
        x,     y - h, 0.0f, 0.0f
    };

    glUseProgram(shader);

    glUniform4f(
        glGetUniformLocation(shader, "hudColor"),
        r, g, b, a
    );

    glUniform1i(
        glGetUniformLocation(shader, "shapeType"),
        0
    );

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(vertices),
        vertices
    );

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void HudSystem::drawHeart(
    float x,
    float y,
    float size
) {
    float vertices[] = {
        // pos x, pos y, uv x, uv y
        x,        y,        0.0f, 1.0f,
        x + size, y,        1.0f, 1.0f,
        x + size, y - size, 1.0f, 0.0f,

        x,        y,        0.0f, 1.0f,
        x + size, y - size, 1.0f, 0.0f,
        x,        y - size, 0.0f, 0.0f
    };

    glUseProgram(shader);

    glUniform4f(
        glGetUniformLocation(shader, "hudColor"),
        1.0f, 0.05f, 0.15f, 1.0f
    );

    glUniform1i(
        glGetUniformLocation(shader, "shapeType"),
        1
    );

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        sizeof(vertices),
        vertices
    );

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void HudSystem::draw(
    int lives,
    bool fastBulletsActive,
    float fastBulletsTimeLeft
) {
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Hearts in top-left corner
    for (int i = 0; i < lives; i++) {
        float x = -0.95f + i * 0.08f;
        drawHeart(x, 0.96f, 0.065f);
    }

    // Fast bullets bar in top-right corner
    if (fastBulletsActive) {
        float maxTime = 8.0f;
        float ratio = fastBulletsTimeLeft / maxTime;

        if (ratio < 0.0f) {
            ratio = 0.0f;
        }

        if (ratio > 1.0f) {
            ratio = 1.0f;
        }

        // background
        drawRect(
            0.55f,
            0.94f,
            0.35f,
            0.035f,
            0.15f,
            0.15f,
            0.22f,
            0.85f
        );

        // active cyan bar
        drawRect(
            0.55f,
            0.94f,
            0.35f * ratio,
            0.035f,
            0.2f,
            0.9f,
            1.0f,
            1.0f
        );
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HudSystem::cleanup() {
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    if (vbo != 0) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if (shader != 0) {
        glDeleteProgram(shader);
        shader = 0;
    }
}