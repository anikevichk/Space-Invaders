#pragma once

#include <GL/glew.h>
#include <string>

GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);