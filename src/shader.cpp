#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/shader.hpp"


// Construct a shader program from vertex and fragment shader files
Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath){
    int success;
    char infoLog[512];

    // Print shader file paths for debugging
    std::cout << vertexShaderPath << std::endl << fragmentShaderPath << std::endl;

    // Compile vertex and fragment shaders
    GLuint vertexShader = compileShader(vertexShaderPath, GL_VERTEX_SHADER);
    GLuint fragShader = compileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

    // Create shader program and link shaders
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragShader);
    glLinkProgram(ID);

    // Check for linking errors
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    // Shaders can be deleted after linking
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
}

// Activate the shader program for rendering
void Shader::activate(){ glUseProgram(ID);}

// Load shader source code from a file
std::string Shader::loadShaderSrc(const char* src){
    std::ifstream file;
    std::stringstream buf;
    //std::cout << "Current root path is: " << std::filesystem::current_path()<< '\n';
    file.open(src);
    if(file.is_open()){
        buf << file.rdbuf();
    } else {
        std::cout << "Could not open " << src << std::endl;
    }
    file.close();
    return buf.str();
}

// Compile a shader from source file (vertex or fragment)
GLuint Shader::compileShader(const char* filepath, GLenum type){
    int success;
    char infoLog[512];

    GLuint ret = glCreateShader(type);
    std::string shaderSrc = loadShaderSrc(filepath);
    const GLchar* shader = shaderSrc.c_str();
    glShaderSource(ret, 1, &shader, NULL);
    glCompileShader(ret);

    // Check for compilation errors
    glGetShaderiv(ret, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(ret, 512, NULL, infoLog);
        std::cout << "Error with fragment shader compi.:\n" << infoLog << std::endl;
    }

    return ret;
}

// Set a 4x4 matrix uniform in the shader
void Shader::setMat4(const std::string& name, glm::mat4 val){
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
}

void Shader::setVec4(const std::string& name, const glm::vec4& val) {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(val));
}

void Shader::setVec2(const std::string& name, const glm::vec2& val) {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(val));
}
