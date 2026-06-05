#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aTrail;

uniform mat4 view;
uniform mat4 projection;

out float vTrail;

void main() {
    vTrail = aTrail;
    gl_Position = projection * view * vec4(aPos, 1.0);
}
