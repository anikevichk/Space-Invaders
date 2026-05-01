#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    Camera(float width, float height);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

private:
    float width;
    float height;
};