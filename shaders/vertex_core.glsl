#version 330 core

// Input vertex data (position)
layout (location = 0) in vec3 aPos;

// Uniforms are global variables passed from C++ to the shader
// This matrix will contain our combined model, view, and projection transformations
uniform mat4 mvp;

void main()
{
    // Transform the vertex position and output it
    gl_Position = mvp * vec4(aPos, 1.0);
}
