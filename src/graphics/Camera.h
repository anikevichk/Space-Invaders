#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    Camera(float width, float height);

    void setSize(float newWidth, float newHeight);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    float getWorldHalfWidth() const;
    float getPlayerMinX() const;
    float getPlayerMaxX() const;

private:
    float width;
    float height;

    const float worldHalfHeight = 3.0f;
};