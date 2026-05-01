#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D shipTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec3 objectColor = texture(shipTexture, TexCoord).rgb;
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.35;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;

    // --- emissive for pink parts ---
    float pinkMask = 0.0;
    if (objectColor.r > 0.75 && objectColor.b > 0.45 && objectColor.g < 0.35) {
        pinkMask = 1.0;
    }

    vec3 emissiveColor = vec3(1.0, 0.1, 0.8) * pinkMask * 2.5;

    result += emissiveColor;

    FragColor = vec4(result, 1.0);
}