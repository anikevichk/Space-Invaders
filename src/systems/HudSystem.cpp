#include "HudSystem.h"

#include <algorithm>
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

void HudSystem::setScreenSize(int width, int height) {
    screenWidth = width > 1 ? width : 1;
    screenHeight = height > 1 ? height : 1;
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

void HudSystem::drawRectPx(
    float x,
    float y,
    float w,
    float h,
    float r,
    float g,
    float b,
    float a
) {
    float ndcX = (x / static_cast<float>(screenWidth)) * 2.0f - 1.0f;
    float ndcY = 1.0f - (y / static_cast<float>(screenHeight)) * 2.0f;
    float ndcW = (w / static_cast<float>(screenWidth)) * 2.0f;
    float ndcH = (h / static_cast<float>(screenHeight)) * 2.0f;

    drawRect(ndcX, ndcY, ndcW, ndcH, r, g, b, a);
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

void HudSystem::drawHeartPx(
    float x,
    float y,
    float size
) {
    float ndcX = (x / static_cast<float>(screenWidth)) * 2.0f - 1.0f;
    float ndcY = 1.0f - (y / static_cast<float>(screenHeight)) * 2.0f;
    float ndcSizeX = (size / static_cast<float>(screenWidth)) * 2.0f;
    float ndcSizeY = (size / static_cast<float>(screenHeight)) * 2.0f;

    float vertices[] = {
        ndcX,            ndcY,            0.0f, 1.0f,
        ndcX + ndcSizeX, ndcY,            1.0f, 1.0f,
        ndcX + ndcSizeX, ndcY - ndcSizeY, 1.0f, 0.0f,

        ndcX,            ndcY,            0.0f, 1.0f,
        ndcX + ndcSizeX, ndcY - ndcSizeY, 1.0f, 0.0f,
        ndcX,            ndcY - ndcSizeY, 0.0f, 0.0f
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
    float fastBulletsTimeLeft,
    int score
) {
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Hearts in top-left corner - pixel based, so they do not stretch on wide screens
    const float heartSize = std::max(24.0f, std::min(screenWidth, screenHeight) * 0.045f);
    const float heartGap = heartSize * 0.25f;

    for (int i = 0; i < lives; i++) {
        float x = 18.0f + i * (heartSize + heartGap);
        drawHeartPx(x, 18.0f, heartSize);
    }

    const float minSide = static_cast<float>(std::min(screenWidth, screenHeight));
    const float scorePixel = std::max(2.6f, std::min(minSide * 0.0046f, 4.0f));
    drawCenteredTextPx(
        "SCORE " + std::to_string(score),
        static_cast<float>(screenWidth) * 0.5f,
        18.0f,
        scorePixel,
        0.85f, 0.93f, 1.0f, 1.0f
    );

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

static bool glyphPixel(char c, int row, int col) {
    static const char* SPACE[7] = {
        "00000", "00000", "00000", "00000", "00000", "00000", "00000"
    };
    static const char* A[7] = {"01110", "10001", "10001", "11111", "10001", "10001", "10001"};
    static const char* D[7] = {"11110", "10001", "10001", "10001", "10001", "10001", "11110"};
    static const char* E[7] = {"11111", "10000", "10000", "11110", "10000", "10000", "11111"};
    static const char* G[7] = {"01111", "10000", "10000", "10011", "10001", "10001", "01111"};
    static const char* I[7] = {"11111", "00100", "00100", "00100", "00100", "00100", "11111"};
    static const char* M[7] = {"10001", "11011", "10101", "10101", "10001", "10001", "10001"};
    static const char* N[7] = {"10001", "11001", "10101", "10011", "10001", "10001", "10001"};
    static const char* O[7] = {"01110", "10001", "10001", "10001", "10001", "10001", "01110"};
    static const char* P[7] = {"11110", "10001", "10001", "11110", "10000", "10000", "10000"};
    static const char* R[7] = {"11110", "10001", "10001", "11110", "10100", "10010", "10001"};
    static const char* S[7] = {"01111", "10000", "10000", "01110", "00001", "00001", "11110"};
    static const char* T[7] = {"11111", "00100", "00100", "00100", "00100", "00100", "00100"};
    static const char* V[7] = {"10001", "10001", "10001", "10001", "10001", "01010", "00100"};
    static const char* W[7] = {"10001", "10001", "10001", "10101", "10101", "10101", "01010"};
    static const char* Y[7] = {"10001", "10001", "01010", "00100", "00100", "00100", "00100"};
    static const char* U[7] = {"10001", "10001", "10001", "10001", "10001", "10001", "01110"};
    static const char* L[7] = {"10000", "10000", "10000", "10000", "10000", "10000", "11111"};
    static const char* H[7] = {"10001", "10001", "10001", "11111", "10001", "10001", "10001"};
    static const char* F[7] = {"11111", "10000", "10000", "11110", "10000", "10000", "10000"};
    static const char* K[7] = {"10001", "10010", "10100", "11000", "10100", "10010", "10001"};
    static const char* C[7] = {"01111", "10000", "10000", "10000", "10000", "10000", "01111"};
    static const char* B[7] = {"11110", "10001", "10001", "11110", "10001", "10001", "11110"};
    static const char* Q[7] = {"01110", "10001", "10001", "10001", "10101", "10010", "01101"};
    static const char* X[7] = {"10001", "10001", "01010", "00100", "01010", "10001", "10001"};
    static const char* Z[7] = {"11111", "00001", "00010", "00100", "01000", "10000", "11111"};
    static const char* DASH[7] = {"00000", "00000", "00000", "11111", "00000", "00000", "00000"};
    static const char* COLON[7] = {"00000", "00100", "00000", "00000", "00000", "00100", "00000"};
    static const char* ZERO[7]  = {"01110", "10001", "10011", "10101", "11001", "10001", "01110"};
    static const char* ONE[7]   = {"00100", "01100", "00100", "00100", "00100", "00100", "01110"};
    static const char* TWO[7]   = {"01110", "10001", "00001", "00010", "00100", "01000", "11111"};
    static const char* THREE[7] = {"11110", "00001", "00001", "01110", "00001", "00001", "11110"};
    static const char* FOUR[7]  = {"00010", "00110", "01010", "10010", "11111", "00010", "00010"};
    static const char* FIVE[7]  = {"11111", "10000", "11110", "00001", "00001", "10001", "01110"};
    static const char* SIX[7]   = {"00110", "01000", "10000", "11110", "10001", "10001", "01110"};
    static const char* SEVEN[7] = {"11111", "00001", "00010", "00100", "01000", "01000", "01000"};
    static const char* EIGHT[7] = {"01110", "10001", "10001", "01110", "10001", "10001", "01110"};
    static const char* NINE[7]  = {"01110", "10001", "10001", "01111", "00001", "00010", "01100"};

    const char** pattern = SPACE;

    switch (c) {
        case 'A': pattern = A; break;
        case 'B': pattern = B; break;
        case 'C': pattern = C; break;
        case 'D': pattern = D; break;
        case 'E': pattern = E; break;
        case 'F': pattern = F; break;
        case 'G': pattern = G; break;
        case 'H': pattern = H; break;
        case 'I': pattern = I; break;
        case 'K': pattern = K; break;
        case 'L': pattern = L; break;
        case 'M': pattern = M; break;
        case 'N': pattern = N; break;
        case 'O': pattern = O; break;
        case 'P': pattern = P; break;
        case 'Q': pattern = Q; break;
        case 'R': pattern = R; break;
        case 'S': pattern = S; break;
        case 'T': pattern = T; break;
        case 'U': pattern = U; break;
        case 'V': pattern = V; break;
        case 'W': pattern = W; break;
        case 'X': pattern = X; break;
        case 'Y': pattern = Y; break;
        case 'Z': pattern = Z; break;
        case '-': pattern = DASH; break;
        case ':': pattern = COLON; break;
        case '0': pattern = ZERO; break;
        case '1': pattern = ONE; break;
        case '2': pattern = TWO; break;
        case '3': pattern = THREE; break;
        case '4': pattern = FOUR; break;
        case '5': pattern = FIVE; break;
        case '6': pattern = SIX; break;
        case '7': pattern = SEVEN; break;
        case '8': pattern = EIGHT; break;
        case '9': pattern = NINE; break;
        default: pattern = SPACE; break;
    }

    return pattern[row][col] == '1';
}

void HudSystem::drawText(
    const std::string& text,
    float x,
    float y,
    float scale,
    float r,
    float g,
    float b,
    float a
) {
    const float pixel = scale;
    const float gap = scale;
    const float charWidth = 5.0f * pixel;
    const float charStep = charWidth + gap;

    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        float charX = x + static_cast<float>(i) * charStep;

        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (glyphPixel(c, row, col)) {
                    drawRect(
                        charX + static_cast<float>(col) * pixel,
                        y - static_cast<float>(row) * pixel,
                        pixel * 0.88f,
                        pixel * 0.88f,
                        r,
                        g,
                        b,
                        a
                    );
                }
            }
        }
    }
}

void HudSystem::drawTextPx(
    const std::string& text,
    float x,
    float y,
    float pixelSize,
    float r,
    float g,
    float b,
    float a
) {
    const float cell = pixelSize;
    const float charStep = 6.0f * cell;

    for (size_t i = 0; i < text.size(); i++) {
        char c = text[i];
        float charX = x + static_cast<float>(i) * charStep;

        for (int row = 0; row < 7; row++) {
            for (int col = 0; col < 5; col++) {
                if (glyphPixel(c, row, col)) {
                    drawRectPx(
                        charX + static_cast<float>(col) * cell,
                        y + static_cast<float>(row) * cell,
                        cell * 0.86f,
                        cell * 0.86f,
                        r,
                        g,
                        b,
                        a
                    );
                }
            }
        }
    }
}

void HudSystem::drawCenteredText(
    const std::string& text,
    float centerX,
    float y,
    float scale,
    float r,
    float g,
    float b,
    float a
) {
    float width = static_cast<float>(text.size()) * 6.0f * scale;
    drawText(text, centerX - width * 0.5f, y, scale, r, g, b, a);
}

void HudSystem::drawCenteredTextPx(
    const std::string& text,
    float centerX,
    float y,
    float pixelSize,
    float r,
    float g,
    float b,
    float a
) {
    float width = static_cast<float>(text.size()) * 6.0f * pixelSize;
    drawTextPx(text, centerX - width * 0.5f, y, pixelSize, r, g, b, a);
}

void HudSystem::drawOverlay(
    const std::string& title,
    float titleR,
    float titleG,
    float titleB,
    const std::string& subtitle,
    int score
) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float minSide = static_cast<float>(std::min(screenWidth, screenHeight));
    const float centerX = static_cast<float>(screenWidth) * 0.5f;
    const float centerY = static_cast<float>(screenHeight) * 0.43f;

    float panelW = std::min(static_cast<float>(screenWidth) * 0.72f, minSide * 1.15f);
    float panelH = std::min(static_cast<float>(screenHeight) * 0.40f, minSide * 0.42f);
    panelW = std::max(panelW, 420.0f);
    panelH = std::max(panelH, 200.0f);

    float panelX = centerX - panelW * 0.5f;
    float panelY = centerY - panelH * 0.5f;

    float titleScale = minSide * 0.010f;
    titleScale = std::max(4.5f, std::min(titleScale, 8.0f));

    float titleMaxW = panelW * 0.82f;
    float titleFit = titleMaxW / (static_cast<float>(title.size()) * 6.0f);
    titleScale = std::min(titleScale, titleFit);

    float subScale = std::max(3.5f, std::min(minSide * 0.0072f, 6.0f));
    float hintScale = std::max(3.0f, std::min(minSide * 0.0058f, 5.0f));

    drawRectPx(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f, 0.0f, 0.03f, 0.34f);

    // Main menu panel: more transparent, so the animated 3D background stays visible.
    drawRectPx(panelX, panelY, panelW, panelH, 0.035f, 0.045f, 0.11f, 0.62f);
    drawRectPx(panelX, panelY, panelW, 4.0f, titleR, titleG, titleB, 0.95f);
    drawRectPx(panelX, panelY + panelH - 4.0f, panelW, 4.0f, titleR, titleG, titleB, 0.95f);

    drawCenteredTextPx(title, centerX, panelY + panelH * 0.20f, titleScale, titleR, titleG, titleB, 1.0f);
    drawCenteredTextPx("SCORE " + std::to_string(score), centerX, panelY + panelH * 0.44f, subScale, 1.0f, 0.92f, 0.45f, 1.0f);
    drawCenteredTextPx(subtitle, centerX, panelY + panelH * 0.62f, hintScale, 0.86f, 0.93f, 1.0f, 1.0f);
    drawCenteredTextPx("ESC - QUIT", centerX, panelY + panelH * 0.78f, hintScale, 0.55f, 0.65f, 0.78f, 1.0f);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HudSystem::drawStartScreen() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float minSide = static_cast<float>(std::min(screenWidth, screenHeight));
    const float centerX = static_cast<float>(screenWidth) * 0.5f;
    const float centerY = static_cast<float>(screenHeight) * 0.47f;

    float panelW = std::max(360.0f, std::min(static_cast<float>(screenWidth) * 0.54f, minSide * 0.86f));
    float panelH = std::max(128.0f, std::min(static_cast<float>(screenHeight) * 0.24f, minSide * 0.28f));

    float panelX = centerX - panelW * 0.5f;
    float panelY = centerY - panelH * 0.5f;

    float titleScale = std::max(3.2f, std::min(minSide * 0.0060f, 5.2f));
    float titleFit = (panelW * 0.80f) / (17.0f * 6.0f);
    titleScale = std::min(titleScale, titleFit);

    float subScale = std::max(3.1f, std::min(minSide * 0.0054f, 4.4f));
    float hintScale = std::max(2.4f, std::min(minSide * 0.0042f, 3.4f));

    drawRectPx(0.0f, 0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f, 0.0f, 0.02f, 0.20f);
    drawRectPx(panelX, panelY, panelW, panelH, 0.02f, 0.05f, 0.12f, 0.48f);
    drawRectPx(panelX, panelY, panelW, 2.0f, 0.25f, 0.92f, 1.0f, 0.92f);
    drawRectPx(panelX, panelY + panelH - 2.0f, panelW, 2.0f, 0.25f, 0.92f, 1.0f, 0.92f);

    drawCenteredTextPx("SPACE INVADERS 3D", centerX, panelY + panelH * 0.30f, titleScale, 0.35f, 0.92f, 1.0f, 1.0f);
    drawCenteredTextPx("PRESS ENTER", centerX, panelY + panelH * 0.56f, subScale, 0.92f, 0.95f, 1.0f, 0.98f);
    drawCenteredTextPx("ESC - QUIT", centerX, panelY + panelH * 0.75f, hintScale, 0.60f, 0.68f, 0.80f, 1.0f);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HudSystem::drawGameOverScreen(int score) {
    drawOverlay("GAME OVER", 1.0f, 0.18f, 0.18f, "PRESS R", score);
}

void HudSystem::drawWinScreen(int score) {
    drawOverlay("YOU WIN", 0.25f, 1.0f, 0.35f, "PRESS R", score);
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