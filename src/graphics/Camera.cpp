#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float width, float height)
    : width(width), height(height) {}

void Camera::setSize(float newWidth, float newHeight) {
    width = newWidth;
    height = newHeight;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(
        glm::vec3(0.0f, 2.0f, 7.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

glm::mat4 Camera::getProjectionMatrix() const {
    float aspect = width / height;

    return glm::perspective(
        glm::radians(45.0f),
        aspect,
        0.1f,
        100.0f
    );
}

float Camera::getWorldHalfWidth() const {
    float aspect = width / height;

    return 3.0f * aspect;
}

float Camera::getPlayerMinX() const {
    return -getWorldHalfWidth() + 0.65f;
}

float Camera::getPlayerMaxX() const {
    return getWorldHalfWidth() - 0.65f;
}