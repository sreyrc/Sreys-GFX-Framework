#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class Shader
{
public:
	Shader(const char* vertexShaderPath, const char* fragmentShaderPath);
	void Use();

	void SetVec3(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2);
	void SetVec3(const std::string& name, glm::vec3 value);
	void SetFloat(const std::string& name, GLfloat value);
	void SetInt(const std::string& name, GLint value);
	void SetMat4(const std::string& name, const glm::mat4& mat);

private:
	GLuint mID;
};

