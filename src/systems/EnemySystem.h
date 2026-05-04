#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <random>

struct EnemyBullet {
    glm::vec3 position;
    glm::vec3 velocity;
};

struct Enemy {
    glm::vec3 basePosition;
    bool      alive = true;
};

class EnemySystem {
public:
    bool init();
    void update(float deltaTime);
    void draw(const glm::mat4& view, const glm::mat4& projection);
    void cleanup();
    void reset();

    bool hitByBullet(const glm::vec3& bulletStart, const glm::vec3& bulletEnd);
    bool playerHit(float playerX, float playerZ);
    bool allDead()   const;
    int  aliveCount() const;

private:
    static constexpr int   COLS        = 11;
    static constexpr int   ROWS        = 5;
    static constexpr float COL_STEP    = 0.50f;
    static constexpr float ROW_STEP_Z  = 0.70f;
    static constexpr float GRID_BASE_X = -2.50f;
    static constexpr float GRID_BASE_Z = -4.00f;
    static constexpr float GRID_Y      = -0.50f;
    static constexpr float ENEMY_SCALE = 0.09f;
    static constexpr float HITBOX_HX   = 0.42f;
    static constexpr float HITBOX_HZ   = 0.38f;
    static constexpr float SPEED_BASE  = 0.80f;
    static constexpr float SPEED_MAX   = 2.50f;
    static constexpr float BOUNDARY_X  = 3.20f;
    static constexpr float ADVANCE_Z   = 0.167f;   // was 0.5, /3
    static constexpr float BULLET_SPEED = 3.5f;
    static constexpr int   TYPE_COUNT  = 3;

    // Each type covers a contiguous range of rows and uses its own model
    struct EnemyType {
        GLuint      vao             = 0;
        GLuint      meshVBO         = 0;
        GLuint      instanceVBO     = 0;
        GLuint      texture         = 0;
        GLsizei     meshVertexCount = 0;
        int         firstRow        = 0;
        int         numRows         = 0;
        glm::vec3   emissiveTint    = glm::vec3(0.0f);
        float       hitboxHX        = 0.0f;  // world-space half-extents after scale
        float       hitboxHZ        = 0.0f;
    };

    std::array<EnemyType, TYPE_COUNT> types;
    GLuint enemyShader  = 0;
    GLuint bulletShader = 0;
    GLuint bulletVAO    = 0;
    GLuint bulletVBO    = 0;

    std::vector<Enemy>       enemies;
    std::vector<EnemyBullet> enemyBullets;

    float gridOffsetX = 0.0f;
    float dirX        = 1.0f;
    float totalTime   = 0.0f;
    float shootTimer  = 0.0f;
    float nextShootIn = 1.5f;

    std::mt19937 rng;

    bool loadType(int typeIdx,
                  const char* objPath, const char* mtlDir,
                  const char* texPath,
                  int firstRow, int numRows,
                  glm::vec3 emissiveTint);

    float     currentSpeed()                                   const;
    glm::vec3 enemyWorldPos(const Enemy& e)                    const;
    bool      bulletHitsEnemy(const glm::vec3& pt, int idx)   const;
    void      shootFromRandom();
    void      buildGrid();
};
