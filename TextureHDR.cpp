#include <glad/glad.h>
#include <iostream>

#include "TextureHDR.h"

TextureHDR::TextureHDR(std::string path) : mID(0) {

	int width, height, nrComponents;
	
	float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);

	if (data) {
		glGenTextures(1, &mID);
		glBindTexture(GL_TEXTURE_2D, mID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
	}
	else {
		std::cout << "Failed to load HDR image." << std::endl;
		std::cout << stbi_failure_reason() << '\n';
	}
}

void TextureHDR::Bind() {
	glBindTexture(GL_TEXTURE_2D, mID);
}

void TextureHDR::Unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint TextureHDR::GetID() {
	return mID;
}
