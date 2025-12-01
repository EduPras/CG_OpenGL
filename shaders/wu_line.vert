#version 330 core
layout (location = 0) in vec2 aPos;   // Position from VBO
layout (location = 1) in vec4 aColor; // Color from VBO (includes Red markers & AA alpha)

out vec4 vertexColor; // Output to Fragment Shader

uniform vec2 u_screenSize;

void main() {
    // Convert screen coordinates to NDC
    float ndc_x = (aPos.x / u_screenSize.x) * 2.0 - 1.0;
    // Note: This assumes (0,0) is Top-Left. 
    // If your projection outputs Bottom-Left, remove the "1.0 -"
    float ndc_y = 1.0 - (aPos.y / u_screenSize.y) * 2.0; 

    gl_Position = vec4(ndc_x, ndc_y, 0.0, 1.0);
    
    // Pass the specific color of THIS vertex to the fragment shader
    vertexColor = aColor; 
}