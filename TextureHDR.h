#pragma once

#include <string>
#include <stb/stb_image.h>

class TextureHDR
{
public:
	TextureHDR(std::string path);
	void Bind();
	void Unbind();

	GLuint GetID();

private:
	GLuint mID;
};

