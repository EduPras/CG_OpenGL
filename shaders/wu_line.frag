#version 330 core
out vec4 FragColor;

// ERROR WAS HERE: Changed 'uniform' to 'in'
in vec4 vertexColor; 

void main() {
    // Now it uses the color passed from the vertex, 
    // allowing Red markers and White lines to exist together.
    FragColor = vertexColor;
}