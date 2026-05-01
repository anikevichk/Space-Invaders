#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float width, float height)
    : width(width), height(height) {}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(
        glm::vec3(0.0f, 2.0f, 7.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(
        glm::radians(45.0f),
        width / height,
        0.1f,
        100.0f
    );
}