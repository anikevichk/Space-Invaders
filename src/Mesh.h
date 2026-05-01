#pragma once

#include <GL/glew.h>
#include <vector>
#include "ObjLoader.h"

struct Mesh {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLsizei vertexCount = 0;
};

Mesh createShipMesh(const std::vector<Vertex>& vertices);
Mesh createPointMesh(const float* points, size_t floatCount);
void destroyMesh(Mesh& mesh);