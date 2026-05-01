#version 330 core

out vec4 FragColor;

void main() {
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    float dist = length(coord);

    float alpha = 1.0 - smoothstep(0.0, 1.0, dist);

    vec3 glowColor = vec3(1.0, 0.05, 0.75);

    FragColor = vec4(glowColor, alpha * 0.55);
}