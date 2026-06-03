#version 330 core

out vec4 FragColor;

uniform vec3 powerUpColor;
uniform int powerUpShape;

// 0 = lightning
// 1 = heart

bool heartPixel(int row, int col) {
    // 9x9 pixel heart
    if (row == 0) return col == 1 || col == 2 || col == 6 || col == 7;
    if (row == 1) return col == 0 || col == 1 || col == 2 || col == 3 || col == 5 || col == 6 || col == 7 || col == 8;
    if (row == 2) return col >= 0 && col <= 8;
    if (row == 3) return col >= 0 && col <= 8;
    if (row == 4) return col >= 1 && col <= 7;
    if (row == 5) return col >= 2 && col <= 6;
    if (row == 6) return col >= 3 && col <= 5;
    if (row == 7) return col == 4;

    return false;
}

bool lightningPixel(int row, int col) {
    // 9x9 pixel lightning
    if (row == 0) return col == 5 || col == 6;
    if (row == 1) return col == 4 || col == 5;
    if (row == 2) return col == 3 || col == 4;
    if (row == 3) return col == 2 || col == 3 || col == 4 || col == 5;
    if (row == 4) return col == 4 || col == 5;
    if (row == 5) return col == 3 || col == 4;
    if (row == 6) return col == 2 || col == 3;
    if (row == 7) return col == 1 || col == 2;

    return false;
}

bool shapePixel(int row, int col) {
    if (powerUpShape == 0) {
        return lightningPixel(row, col);
    }

    return heartPixel(row, col);
}

float shapeGlow(vec2 uv) {
    float minDist = 10.0;

    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            if (shapePixel(r, c)) {
                vec2 center = vec2(
                    (float(c) + 0.5) / 9.0,
                    (float(r) + 0.5) / 9.0
                );

                float d = length(uv - center);
                minDist = min(minDist, d);
            }
        }
    }

    return 1.0 - smoothstep(0.08, 0.28, minDist);
}

void main() {
    vec2 uv = gl_PointCoord;

    int col = int(clamp(uv.x, 0.0, 0.999) * 9.0);

    // row 0 = top
    int row = int(clamp(uv.y, 0.0, 0.999) * 9.0);

    bool pixel = shapePixel(row, col);
    float glow = shapeGlow(uv);

    if (!pixel && glow <= 0.0) {
        discard;
    }

    if (pixel) {
        FragColor = vec4(powerUpColor * 1.7, 1.0);
    }
    else {
        FragColor = vec4(powerUpColor, glow * 0.35);
    }
}