#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>

void checkCompileErrors(GLuint shader, std::string type);

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexShaderCodeString;
    std::string fragmentShaderCodeString;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexShaderPath);
        fShaderFile.open(fragmentShaderPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexShaderCodeString = vShaderStream.str();
        fragmentShaderCodeString = fShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
    }

    GLuint vertexShader, fragmentShader;

    const GLchar* vertexShaderCode = vertexShaderCodeString.c_str();
    const GLchar* fragmentShaderCode = fragmentShaderCodeString.c_str();

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");
    
    mID = glCreateProgram();
    glAttachShader(mID, vertexShader);
    glAttachShader(mID, fragmentShader);
    glLinkProgram(mID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::Use()
{
    glUseProgram(mID);
}

void Shader::SetVec3(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
{
    glUniform3f(glGetUniformLocation(mID, name.c_str()), v0, v1, v2);
}

void Shader::SetVec3(const std::string& name, glm::vec3 value)
{
    glUniform3fv(glGetUniformLocation(mID, name.c_str()), 1, &value[0]);
}

void Shader::SetFloat(const std::string& name, GLfloat value)
{
    glUniform1f(glGetUniformLocation(mID, name.c_str()), value);
}

void Shader::SetInt(const std::string& name, GLint value)
{
    glUniform1i(glGetUniformLocation(mID, name.c_str()), value);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat)
{
    glUniformMatrix4fv(glGetUniformLocation(mID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
