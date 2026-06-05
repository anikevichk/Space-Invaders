#version 330 core

in vec4 particleColor;
out vec4 FragColor;

void main() {
    vec2 centered = gl_PointCoord - vec2(0.5);
    float dist = length(centered);

    if (dist > 0.5) {
        discard;
    }

    float softEdge = 1.0 - smoothstep(0.18, 0.5, dist);
    FragColor = vec4(particleColor.rgb, particleColor.a * softEdge);
}
