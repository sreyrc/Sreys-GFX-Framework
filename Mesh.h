#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include "Texture.h"
#include "Shader.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

class Mesh {
public:
	std::vector<Vertex> mVertices;
	std::vector<GLuint> mIndices;
	std::vector<Texture*> mTextures;

	GLuint mVAO;

	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture*> textures);

	void Draw(Shader* shader);

private:
	GLuint mVBO, mEBO;

	void SetupMesh();
};
