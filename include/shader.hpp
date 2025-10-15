
#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <glm/glm.hpp>

// Forward declarations to avoid including glad in header
typedef unsigned int GLuint;
typedef unsigned int GLenum;

/**
 * @brief Manages OpenGL shader programs.
 *
 * The Shader class loads, compiles, and links vertex and fragment shaders,
 * and provides methods to activate the shader and set uniforms.
 */
class Shader {
private:
    /**
     * @brief OpenGL shader program ID.
     */
    unsigned int ID;

    /**
     * @brief Loads shader source code from a file.
     * @param filepath Path to the shader source file.
     * @return Shader source code as a string.
     */
    std::string loadShaderSrc(const char* filepath);

    /**
     * @brief Compiles a shader from source file.
     * @param filepath Path to the shader source file.
     * @param type OpenGL shader type (GL_VERTEX_SHADER or GL_FRAGMENT_SHADER).
     * @return OpenGL shader object ID.
     */
    GLuint compileShader(const char* filepath, GLenum type);


    

public:
    /**
     * @brief Constructs a shader program from vertex and fragment shader files.
     * @param vertexShaderPath Path to the vertex shader source file.
     * @param fragmentShaderPath Path to the fragment shader source file.
     */
    Shader(const char* vertexShaderPath, const char* fragmentShaderPath);

    /**
     * @brief Activates the shader program for rendering.
     */
    void activate();

    /**
     * @brief Sets a 4x4 matrix uniform in the shader.
     * @param name Name of the uniform variable.
     * @param val Matrix value to set.
     */
    void setMat4(const std::string& name, glm::mat4 val);
    
    /**
     * @brief Sets a vec4 uniform in the shader.
     * @param name Name of the uniform variable.
     * @param val Vector value to set.
     */
    void setVec4(const std::string& name, const glm::vec4& val);

    /**
     * @brief Sets a vec4 uniform in the shader.
     * @param name Name of the uniform variable.
     * @param val Vector value to set.
     */
    void setVec2(const std::string& name, const glm::vec2& val);
};

#endif
