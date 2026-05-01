#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

std::vector<Vertex> loadObjModel(
    const std::string& path,
    const std::string& mtlSearchPath
);