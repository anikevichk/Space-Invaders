#include "EnemySystem.h"
#include "Shader.h"
#include "ObjLoader.h"
#include "Texture.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <ctime>
#include <iostream>
#include <limits>

// ── helpers ──────────────────────────────────────────────────────────────────

static void setupInstanceAttribs(GLuint instanceVBO, int maxInstances) {
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, maxInstances * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

    // mat4 at locations 3..6 (4 × vec4, one per column)
    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(
            3 + i, 4, GL_FLOAT, GL_FALSE,
            sizeof(glm::mat4),
            (void*)(i * sizeof(glm::vec4))
        );
        glEnableVertexAttribArray(3 + i);
        glVertexAttribDivisor(3 + i, 1);
    }
}

// ── public ───────────────────────────────────────────────────────────────────

bool EnemySystem::init() {
    rng.seed(static_cast<unsigned>(time(nullptr)));

    enemyShader = createShaderProgram("shaders/enemy.vert", "shaders/enemy.frag");
    if (!enemyShader) { std::cout << "Enemy shader failed\n"; return false; }

    bulletShader = createShaderProgram("shaders/bullet.vert", "shaders/bullet.frag");
    if (!bulletShader) { std::cout << "Enemy bullet shader failed\n"; return false; }

    // Type 0 — front 2 rows  (closest to player)
    if (!loadType(0,
        "assets/models/UltravioletIntruder/UltravioletIntruder.obj",
        "assets/models/UltravioletIntruder/",
        "assets/models/UltravioletIntruder/UltravioletIntruder.png",
        0, 2, glm::vec3(0.04f, 0.0f, 0.08f)))
        return false;

    // Type 1 — middle 2 rows
    if (!loadType(1,
        "assets/models/CamoStellarJet/CamoStellarJet.obj",
        "assets/models/CamoStellarJet/",
        "assets/models/CamoStellarJet/CamoStellarJet.png",
        2, 2, glm::vec3(0.0f, 0.04f, 0.02f)))
        return false;

    // Type 2 — back row  (furthest from player)
    if (!loadType(2,
        "assets/models/Transtellar/Transtellar.obj",
        "assets/models/Transtellar/",
        "assets/models/Transtellar/Transtellar.png",
        4, 1, glm::vec3(0.06f, 0.02f, 0.0f)))
        return false;

    // Enemy bullet VAO/VBO
    glGenVertexArrays(1, &bulletVAO);
    glGenBuffers(1, &bulletVBO);
    glBindVertexArray(bulletVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bulletVBO);
    glBufferData(GL_ARRAY_BUFFER, 200 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    buildGrid();
    return true;
}

bool EnemySystem::loadType(int idx,
                            const char* objPath, const char* mtlDir,
                            const char* texPath,
                            int firstRow, int numRows,
                            glm::vec3 emissiveTint)
{
    EnemyType& t = types[idx];
    t.firstRow     = firstRow;
    t.numRows      = numRows;
    t.emissiveTint = emissiveTint;

    std::vector<Vertex> verts = loadObjModel(objPath, mtlDir);
    if (verts.empty()) {
        std::cout << "Enemy model not loaded: " << objPath << "\n";
        return false;
    }
    t.meshVertexCount = static_cast<GLsizei>(verts.size());

    // Compute world-space half-extents from actual vertex data
    glm::vec3 vmin(std::numeric_limits<float>::max());
    glm::vec3 vmax(-std::numeric_limits<float>::max());
    for (const auto& v : verts) {
        vmin = glm::min(vmin, v.position);
        vmax = glm::max(vmax, v.position);
    }
    t.hitboxHX = (vmax.x - vmin.x) * 0.5f * ENEMY_SCALE;
    t.hitboxHZ = (vmax.z - vmin.z) * 0.5f * ENEMY_SCALE;

    t.texture = loadTexture(texPath);
    if (!t.texture) {
        std::cout << "Enemy texture not loaded: " << texPath << "\n";
        return false;
    }

    glGenVertexArrays(1, &t.vao);
    glGenBuffers(1, &t.meshVBO);
    glGenBuffers(1, &t.instanceVBO);

    glBindVertexArray(t.vao);

    // Mesh geometry (pos=0, normal=1, texcoord=2)
    glBindBuffer(GL_ARRAY_BUFFER, t.meshVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    // Instance mat4 (locations 3..6, divisor=1)
    setupInstanceAttribs(t.instanceVBO, numRows * COLS);

    glBindVertexArray(0);
    return true;
}

void EnemySystem::buildGrid() {
    enemies.clear();
    enemies.reserve(ROWS * COLS);
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            Enemy e;
            e.basePosition = glm::vec3(
                GRID_BASE_X + col * COL_STEP,
                GRID_Y,
                GRID_BASE_Z - row * ROW_STEP_Z
            );
            e.alive = true;
            enemies.push_back(e);
        }
    }
    gridOffsetX = 0.0f;
    dirX        = 1.0f;
    totalTime   = 0.0f;
    shootTimer  = 0.0f;
    nextShootIn = 1.5f;
    enemyBullets.clear();
}

void EnemySystem::reset() { buildGrid(); }

// ── update ───────────────────────────────────────────────────────────────────

float EnemySystem::currentSpeed() const {
    int alive = aliveCount();
    float ratio = static_cast<float>(alive) / static_cast<float>(ROWS * COLS);
    return SPEED_BASE + (1.0f - ratio) * (SPEED_MAX - SPEED_BASE);
}

int EnemySystem::aliveCount() const {
    int n = 0;
    for (const auto& e : enemies) if (e.alive) n++;
    return n;
}

bool EnemySystem::allDead() const { return aliveCount() == 0; }

glm::vec3 EnemySystem::enemyWorldPos(const Enemy& e) const {
    return { e.basePosition.x + gridOffsetX, e.basePosition.y, e.basePosition.z };
}

bool EnemySystem::bulletHitsEnemy(const glm::vec3& pt, int idx) const {
    int row = idx / COLS;
    const EnemyType* type = nullptr;
    for (const auto& t : types) {
        if (row >= t.firstRow && row < t.firstRow + t.numRows) {
            type = &t;
            break;
        }
    }
    float hx = type ? type->hitboxHX : HITBOX_HX;
    float hz = type ? type->hitboxHZ : HITBOX_HZ;

    glm::vec3 pos = enemyWorldPos(enemies[idx]);
    return std::abs(pt.x - pos.x) <= hx &&
           std::abs(pt.z - pos.z) <= hz;
}

void EnemySystem::shootFromRandom() {
    std::vector<int> shooters;
    shooters.reserve(COLS);
    for (int col = 0; col < COLS; col++) {
        int best = -1;
        for (int row = 0; row < ROWS; row++) {
            int idx = row * COLS + col;
            if (!enemies[idx].alive) continue;
            if (best == -1 ||
                enemies[idx].basePosition.z > enemies[best].basePosition.z)
                best = idx;
        }
        if (best != -1) shooters.push_back(best);
    }
    if (shooters.empty()) return;

    std::uniform_int_distribution<int> pick(0, static_cast<int>(shooters.size()) - 1);
    int idx = shooters[pick(rng)];

    EnemyBullet b;
    b.position = enemyWorldPos(enemies[idx]);
    b.velocity = glm::vec3(0.0f, 0.0f, BULLET_SPEED);
    enemyBullets.push_back(b);
}

void EnemySystem::update(float deltaTime) {
    if (allDead()) return;

    totalTime   += deltaTime;
    gridOffsetX += dirX * currentSpeed() * deltaTime;

    float minX =  std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    for (const auto& e : enemies) {
        if (!e.alive) continue;
        float wx = e.basePosition.x + gridOffsetX;
        if (wx < minX) minX = wx;
        if (wx > maxX) maxX = wx;
    }

    bool advance = false;
    if (dirX > 0.0f && maxX + HITBOX_HX >= BOUNDARY_X) {
        dirX    = -1.0f;
        advance = true;
    } else if (dirX < 0.0f && minX - HITBOX_HX <= -BOUNDARY_X) {
        dirX    = 1.0f;
        advance = true;
    }

    if (advance) {
        for (auto& e : enemies)
            e.basePosition.z += ADVANCE_Z;
    }

    shootTimer += deltaTime;
    if (shootTimer >= nextShootIn) {
        shootTimer = 0.0f;
        std::uniform_real_distribution<float> interval(0.5f, 2.5f);
        nextShootIn = interval(rng);
        shootFromRandom();
    }

    for (auto& b : enemyBullets)
        b.position += b.velocity * deltaTime;

    enemyBullets.erase(
        std::remove_if(enemyBullets.begin(), enemyBullets.end(),
            [](const EnemyBullet& b) { return b.position.z > 5.0f; }),
        enemyBullets.end()
    );
}

bool EnemySystem::hitByBullet(const glm::vec3& bulletStart, const glm::vec3& bulletEnd) {
    constexpr int STEPS = 12;
    for (int i = 0; i < static_cast<int>(enemies.size()); i++) {
        if (!enemies[i].alive) continue;
        for (int s = 0; s <= STEPS; s++) {
            float t  = static_cast<float>(s) / static_cast<float>(STEPS);
            glm::vec3 pt = bulletStart + (bulletEnd - bulletStart) * t;
            if (bulletHitsEnemy(pt, i)) {
                enemies[i].alive = false;
                return true;
            }
        }
    }
    return false;
}

bool EnemySystem::playerHit(float playerX, float playerZ) {
    bool hit = false;
    enemyBullets.erase(
        std::remove_if(enemyBullets.begin(), enemyBullets.end(),
            [&](const EnemyBullet& b) {
                if (std::abs(b.position.x - playerX) < 0.45f &&
                    std::abs(b.position.z - playerZ) < 0.50f) {
                    hit = true;
                    return true;
                }
                return false;
            }),
        enemyBullets.end()
    );
    return hit;
}

// ── draw ─────────────────────────────────────────────────────────────────────

void EnemySystem::draw(const glm::mat4& view, const glm::mat4& projection) {
    // Enemies rotate 10° around X so their nose faces the player (+Z direction)
    static const glm::mat4 ROT =
        glm::rotate(glm::mat4(1.0f), glm::radians(10.0f), glm::vec3(1, 0, 0));
    static const glm::vec3 SCALE(ENEMY_SCALE);
    static const glm::vec3 DEAD_POS(0.0f, 0.0f, 200.0f);

    glUseProgram(enemyShader);
    glUniformMatrix4fv(glGetUniformLocation(enemyShader, "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(enemyShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(enemyShader, "lightPos"),  2.0f, 5.0f, 4.0f);
    glUniform3f(glGetUniformLocation(enemyShader, "viewPos"),   0.0f, 2.0f, 7.0f);
    glUniform1i(glGetUniformLocation(enemyShader, "enemyTexture"), 0);

    for (int t = 0; t < TYPE_COUNT; t++) {
        EnemyType& type = types[t];
        int count = type.numRows * COLS;
        int base  = type.firstRow * COLS;

        std::vector<glm::mat4> matrices(count);
        for (int i = 0; i < count; i++) {
            int ei = base + i;
            if (enemies[ei].alive) {
                glm::mat4 m = glm::translate(glm::mat4(1.0f), enemyWorldPos(enemies[ei]));
                m = glm::scale(m, SCALE);
                m = m * ROT;
                matrices[i] = m;
            } else {
                matrices[i] = glm::translate(glm::mat4(1.0f), DEAD_POS);
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, type.instanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(glm::mat4), matrices.data());

        glUniform3fv(glGetUniformLocation(enemyShader, "emissiveTint"), 1,
                     glm::value_ptr(type.emissiveTint));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, type.texture);

        glBindVertexArray(type.vao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, type.meshVertexCount, count);
        glBindVertexArray(0);
    }

    // ── Enemy bullets ────────────────────────────────────────────────────────
    if (enemyBullets.empty()) return;

    std::vector<glm::vec3> linePoints;
    linePoints.reserve(enemyBullets.size() * 2);
    for (const auto& b : enemyBullets) {
        linePoints.push_back(b.position);
        linePoints.push_back(b.position + glm::vec3(0.0f, 0.0f, 0.35f));
    }

    glUseProgram(bulletShader);
    glUniformMatrix4fv(glGetUniformLocation(bulletShader, "view"),       1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(bulletShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(bulletVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bulletVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, linePoints.size() * sizeof(glm::vec3), linePoints.data());

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glUniform3f(glGetUniformLocation(bulletShader, "bulletColor"), 1.0f, 0.25f, 0.0f);
    glUniform1f(glGetUniformLocation(bulletShader, "alpha"), 0.35f);
    glLineWidth(10.0f);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(linePoints.size()));

    glUniform3f(glGetUniformLocation(bulletShader, "bulletColor"), 1.0f, 0.70f, 0.3f);
    glUniform1f(glGetUniformLocation(bulletShader, "alpha"), 1.0f);
    glLineWidth(3.0f);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(linePoints.size()));

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBindVertexArray(0);
}

// ── cleanup ──────────────────────────────────────────────────────────────────

void EnemySystem::cleanup() {
    for (auto& t : types) {
        if (t.vao)         { glDeleteVertexArrays(1, &t.vao);         t.vao         = 0; }
        if (t.meshVBO)     { glDeleteBuffers(1, &t.meshVBO);           t.meshVBO     = 0; }
        if (t.instanceVBO) { glDeleteBuffers(1, &t.instanceVBO);       t.instanceVBO = 0; }
        if (t.texture)     { glDeleteTextures(1, &t.texture);          t.texture     = 0; }
    }
    if (bulletVAO)    { glDeleteVertexArrays(1, &bulletVAO);   bulletVAO    = 0; }
    if (bulletVBO)    { glDeleteBuffers(1, &bulletVBO);         bulletVBO    = 0; }
    if (enemyShader)  { glDeleteProgram(enemyShader);           enemyShader  = 0; }
    if (bulletShader) { glDeleteProgram(bulletShader);          bulletShader = 0; }
    enemies.clear();
    enemyBullets.clear();
}
