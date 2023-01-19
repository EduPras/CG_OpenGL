#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>

#include <string>
#include <glm/glm.hpp>

class Shader {
    private:
    unsigned int ID;
    
    public:
    Shader(const char* vertexShaderPath, const char* fragmentShaderPath);
    void activate();

    std::string loadShaderSrc(const char* filepath);
    GLuint compileShader(const char* filepath, GLenum type);

    void setMat4(const std::string& name, glm::mat4 val);
};

#endif