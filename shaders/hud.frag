#version 330 core

out vec4 FragColor;

in vec2 uv;

uniform vec4 hudColor;
uniform int shapeType;

// 0 = rectangle
// 1 = pixel heart

bool heartPixel(int row, int col) {
    // 9x8 pixel heart, row 0 = top
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

void main() {
    if (shapeType == 1) {
        int col = int(clamp(uv.x, 0.0, 0.999) * 9.0);
        int row = int(clamp(1.0 - uv.y, 0.0, 0.999) * 8.0);

        bool insidePixel = heartPixel(row, col);

        if (!insidePixel) {
            discard;
        }
    }

    FragColor = hudColor;
}