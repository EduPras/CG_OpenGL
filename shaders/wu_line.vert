#version 330 core
layout (location = 0) in vec2 aPos;   // The 2D pixel position (e.g., 450.0, 320.0)
layout (location = 1) in vec4 aColor; // The color with intensity in alpha

out vec4 vertexColor;

// A 'uniform' is a variable you set from your C++ code
uniform vec2 u_screenSize;

void main() {
    // Convert from screen coordinates (0 -> screenSize) to NDC (-1 -> 1)
    float ndc_x = (aPos.x / u_screenSize.x) * 2.0 - 1.0;
    float ndc_y = 1.0 - (aPos.y / u_screenSize.y) * 2.0; // Invert Y-axis

    gl_Position = vec4(ndc_x, ndc_y, 0.0, 1.0);
    vertexColor = aColor;
}