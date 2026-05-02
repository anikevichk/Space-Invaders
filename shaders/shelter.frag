#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

void main() {
    vec3 baseColor = vec3(0.05, 0.85, 0.62);

    vec3 lightDir = normalize(vec3(-0.4, 1.0, 0.7));
    vec3 normal = normalize(Normal);

    float diffuse = max(dot(normal, lightDir), 0.0);
    float lighting = 0.35 + diffuse * 0.65;

    vec3 color = baseColor * lighting;

    FragColor = vec4(color, 1.0);
}