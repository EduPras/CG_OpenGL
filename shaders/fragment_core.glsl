#version 330 core

// Output color for the fragment (pixel)
out vec4 FragColor;

// NEW: A uniform variable to receive color from the C++ application
uniform vec4 ourColor;

void main()
{
    // Set the fragment's color to the value of our uniform
    FragColor = ourColor;
}

