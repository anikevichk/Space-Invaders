#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D enemyTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 emissiveTint;

void main() {
    vec3 objectColor = texture(enemyTexture, TexCoord).rgb;

    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float ambientStrength = 0.40;
    vec3  ambient = ambientStrength * objectColor;

    float diff    = max(dot(norm, lightDir), 0.0);
    vec3  diffuse = diff * objectColor;

    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3  specular  = 0.3 * spec * vec3(1.0);

    vec3 result = ambient + diffuse + specular + emissiveTint;
    FragColor = vec4(result, 1.0);
}
