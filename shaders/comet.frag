#version 330 core

in float vTrail;
out vec4 FragColor;

void main() {
    vec3 headColor = vec3(0.95, 0.95, 1.0);
    vec3 tailColor = vec3(0.25, 0.7, 1.0);
    vec3 color = mix(tailColor, headColor, vTrail);
    float alpha = mix(0.0, 0.95, vTrail);
    FragColor = vec4(color, alpha);
}
