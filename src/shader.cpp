#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/shader.hpp"

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath){
    int success;
    char infoLog[512];

    std::cout << vertexShaderPath << std::endl << fragmentShaderPath << std::endl;

    GLuint vertexShader = compileShader(vertexShaderPath, GL_VERTEX_SHADER);
    GLuint fragShader = compileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragShader);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragShader);
}

void Shader::activate(){ glUseProgram(ID);}

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

GLuint Shader::compileShader(const char* filepath, GLenum type){
    int success;
    char infoLog[512];

    GLuint ret = glCreateShader(type);
    std::string shaderSrc = loadShaderSrc(filepath);
    const GLchar* shader = shaderSrc.c_str();
    glShaderSource(ret, 1, &shader, NULL);
    glCompileShader(ret);

    glGetShaderiv(ret, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(ret, 512, NULL, infoLog);
        std::cout << "Error with fragment shader compi.:\n" << infoLog << std::endl;
    }

    return ret;
}

void Shader::setMat4(const std::string& name, glm::mat4 val){
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(val));
}