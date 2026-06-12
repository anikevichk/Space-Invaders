#include "EnemySystem.h"
#include "Shader.h"
#include "ObjLoader.h"
#include "Texture.h"
#include "ShelterSystem.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>
#include <limits>
static constexpr float BULLET_RENDER_Y = -1.65f;

// Camera is hardcoded in Camera::getViewMatrix().  The projectile is rendered on
// a lower Y plane than the enemies, so a bullet that visually crosses an enemy
// has different world-space Z than the enemy.  This helper maps a rendered
// bullet point onto the enemy visual plane along the camera ray, making
// collisions match what the player actually sees on screen.
static const glm::vec3 CAMERA_EYE(0.0f, 2.0f, 7.0f);

static glm::vec3 mapRenderedBulletPointToYPlane(
    const glm::vec3& renderedPoint,
    float targetY
) {
    const float denom = renderedPoint.y - CAMERA_EYE.y;

    if (std::abs(denom) < 0.00001f) {
        return renderedPoint;
    }

    const float t = (targetY - CAMERA_EYE.y) / denom;

    return CAMERA_EYE + (renderedPoint - CAMERA_EYE) * t;
}

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

    // Collision must use the same transform as rendering.
    // The OBJ origins are not guaranteed to be in the visual center of the mesh.
    // If we center the hitbox on enemyWorldPos only, shots near the outside columns
    // can look correct on screen but miss the real hitbox.
    glm::vec3 vmin(std::numeric_limits<float>::max());
    glm::vec3 vmax(-std::numeric_limits<float>::max());
    for (const auto& v : verts) {
        vmin = glm::min(vmin, v.position);
        vmax = glm::max(vmax, v.position);
    }

    static const glm::mat4 ROT =
        glm::rotate(glm::mat4(1.0f), glm::radians(10.0f), glm::vec3(1, 0, 0));
    const glm::mat4 localToEnemy =
        glm::scale(glm::mat4(1.0f), glm::vec3(ENEMY_SCALE)) * ROT;

    glm::vec3 hitMin(std::numeric_limits<float>::max());
    glm::vec3 hitMax(-std::numeric_limits<float>::max());

    for (int x = 0; x < 2; ++x) {
        for (int y = 0; y < 2; ++y) {
            for (int z = 0; z < 2; ++z) {
                glm::vec3 corner(
                    x == 0 ? vmin.x : vmax.x,
                    y == 0 ? vmin.y : vmax.y,
                    z == 0 ? vmin.z : vmax.z
                );

                glm::vec3 transformed =
                    glm::vec3(localToEnemy * glm::vec4(corner, 1.0f));

                hitMin = glm::min(hitMin, transformed);
                hitMax = glm::max(hitMax, transformed);
            }
        }
    }

    t.hitboxCenterOffset = (hitMin + hitMax) * 0.5f;
    t.hitboxHX = (hitMax.x - hitMin.x) * 0.5f;
    t.hitboxHZ = (hitMax.z - hitMin.z) * 0.5f;

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
    nextShootIn = 0.55f;
    score       = 0;
    enemyBullets.clear();
}

static int pointsForRow(int row) {
    if (row >= 4) return 30;
    if (row >= 2) return 20;
    return 10;
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

int EnemySystem::getScore() const { return score; }

glm::vec3 EnemySystem::enemyWorldPos(const Enemy& e) const {
    return { e.basePosition.x + gridOffsetX, e.basePosition.y, e.basePosition.z };
}

bool EnemySystem::bulletHitsEnemy(const glm::vec3& pt, int idx) const {
    int row = idx / COLS;

    int typeIdx = 0;
    for (int t = 0; t < TYPE_COUNT; t++) {
        if (row >= types[t].firstRow &&
            row < types[t].firstRow + types[t].numRows) {
            typeIdx = t;
            break;
        }
    }

    const EnemyType& type = types[typeIdx];
    glm::vec3 pos = enemyWorldPos(enemies[idx]) + type.hitboxCenterOffset;

    float hx = type.hitboxHX + 0.12f;
    float hz = type.hitboxHZ + 0.12f;

    return std::abs(pt.x - pos.x) <= hx &&
           std::abs(pt.z - pos.z) <= hz;
}

bool EnemySystem::bulletSegmentHitsEnemy(
    const glm::vec3& start,
    const glm::vec3& end,
    int idx
) const {
    int row = idx / COLS;

    int typeIdx = 0;
    for (int t = 0; t < TYPE_COUNT; t++) {
        if (row >= types[t].firstRow &&
            row < types[t].firstRow + types[t].numRows) {
            typeIdx = t;
            break;
        }
    }

    const EnemyType& type = types[typeIdx];
    glm::vec3 pos = enemyWorldPos(enemies[idx]) + type.hitboxCenterOffset;

    // Bullet positions are rendered at BULLET_RENDER_Y, while enemies are rendered
    // around GRID_Y.  With a perspective camera this creates parallax: the same
    // on-screen bullet point corresponds to a different world X/Z on the enemy
    // plane.  Check collision using that on-screen equivalent, not the raw bullet
    // X/Z, otherwise edge shots look like they pass through enemies.
    const glm::vec3 mappedStart = mapRenderedBulletPointToYPlane(start, pos.y);
    const glm::vec3 mappedEnd   = mapRenderedBulletPointToYPlane(end,   pos.y);

    const float minX = pos.x - type.hitboxHX - 0.12f;
    const float maxX = pos.x + type.hitboxHX + 0.12f;
    const float minZ = pos.z - type.hitboxHZ - 0.12f;
    const float maxZ = pos.z + type.hitboxHZ + 0.12f;

    float tMin = 0.0f;
    float tMax = 1.0f;

    auto clipAxis = [&](float axisStart, float axisEnd, float axisMin, float axisMax) {
        const float d = axisEnd - axisStart;

        if (std::abs(d) < 0.00001f) {
            return axisStart >= axisMin && axisStart <= axisMax;
        }

        float enter = (axisMin - axisStart) / d;
        float exit  = (axisMax - axisStart) / d;

        if (enter > exit) {
            std::swap(enter, exit);
        }

        tMin = std::max(tMin, enter);
        tMax = std::min(tMax, exit);

        return tMin <= tMax;
    };

    return clipAxis(mappedStart.x, mappedEnd.x, minX, maxX) &&
           clipAxis(mappedStart.z, mappedEnd.z, minZ, maxZ);
}

void EnemySystem::shootFromRandom() {
    std::vector<int> shooters;
    shooters.reserve(COLS);

    for (int col = 0; col < COLS; col++) {
        int best = -1;

        for (int row = 0; row < ROWS; row++) {
            int idx = row * COLS + col;

            if (!enemies[idx].alive) {
                continue;
            }

            if (best == -1 || enemies[idx].basePosition.z > enemies[best].basePosition.z) {
                best = idx;
            }
        }

        if (best != -1) {
            shooters.push_back(best);
        }
    }

    if (shooters.empty()) {
        return;
    }

    std::uniform_int_distribution<int> pick(
        0,
        static_cast<int>(shooters.size()) - 1
    );

    int idx = shooters[pick(rng)];

    glm::vec3 enemyPos = enemyWorldPos(enemies[idx]);

    EnemyBullet b;

    b.position = glm::vec3(
        enemyPos.x,
        BULLET_RENDER_Y,
        enemyPos.z - 4.45f
    );

    b.prevPosition = b.position;
    b.velocity = glm::vec3(0.0f, 0.0f, BULLET_SPEED);

    enemyBullets.push_back(b);
}

void EnemySystem::update(float deltaTime, float minBoundaryX, float maxBoundaryX, ShelterSystem* shelterSystem) {
    if (allDead()) {
        return;
    }

    totalTime += deltaTime;
    gridOffsetX += dirX * currentSpeed() * deltaTime;

    float minX = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();

    for (const auto& e : enemies) {
        if (!e.alive) {
            continue;
        }

        float wx = e.basePosition.x + gridOffsetX;

        if (wx < minX) {
            minX = wx;
        }

        if (wx > maxX) {
            maxX = wx;
        }
    }

    if (dirX > 0.0f && maxX + HITBOX_HX >= maxBoundaryX) {
        dirX = -1.0f;
    } else if (dirX < 0.0f && minX - HITBOX_HX <= minBoundaryX) {
        dirX = 1.0f;
    }

    shootTimer += deltaTime;

    if (shootTimer >= nextShootIn) {
        shootTimer = 0.0f;

        std::uniform_real_distribution<float> interval(0.35f, 1.35f);
        nextShootIn = interval(rng);

        shootFromRandom();
    }

    for (EnemyBullet& b : enemyBullets) {
        b.prevPosition = b.position;
        b.position += b.velocity * deltaTime;
    }

    enemyBullets.erase(
        std::remove_if(
            enemyBullets.begin(),
            enemyBullets.end(),
            [&](const EnemyBullet& b) {
                bool outOfBounds = b.position.z > 1.2f;

                bool hitShelter =
                    shelterSystem != nullptr &&
                    shelterSystem->hitByBullet(b.position, b.position);

                return outOfBounds || hitShelter;
            }
        ),
        enemyBullets.end()
    );
}

bool EnemySystem::hitByBullet(
    const glm::vec3& bulletStart,
    const glm::vec3& bulletEnd,
    glm::vec3* killedEnemyPosition
) {
    for (int i = 0; i < static_cast<int>(enemies.size()); i++) {
        if (!enemies[i].alive) {
            continue;
        }

        if (bulletSegmentHitsEnemy(bulletStart, bulletEnd, i)) {
            int row = i / COLS;
            int typeIdx = 0;

            for (int t = 0; t < TYPE_COUNT; t++) {
                if (row >= types[t].firstRow &&
                    row < types[t].firstRow + types[t].numRows) {
                    typeIdx = t;
                    break;
                }
            }

            if (killedEnemyPosition != nullptr) {
                *killedEnemyPosition =
                    enemyWorldPos(enemies[i]) + types[typeIdx].hitboxCenterOffset;
            }

            enemies[i].alive = false;
            score += pointsForRow(row);
            return true;
        }
    }

    return false;
}

bool EnemySystem::playerHit(float playerX, float playerZ) {
    static constexpr float PLAYER_HITBOX_X = 0.75f;
    static constexpr float PLAYER_HITBOX_Z = 0.35f;

    bool hit = false;

    enemyBullets.erase(
        std::remove_if(
            enemyBullets.begin(),
            enemyBullets.end(),
            [&](const EnemyBullet& b) {
                bool bulletHitsPlayer =
                    std::abs(b.position.x - playerX) <= PLAYER_HITBOX_X &&
                    std::abs(b.position.z - playerZ) <= PLAYER_HITBOX_Z;

                if (bulletHitsPlayer) {
                    hit = true;
                    return true;
                }

                return false;
            }
        ),
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
