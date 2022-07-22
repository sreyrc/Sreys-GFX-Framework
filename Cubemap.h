#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <glad/glad.h>

#include "Texture.h"
#include <stb/stb_image.h>


class Cubemap {
public:
    Cubemap(std::vector<std::string>& faces);
    void Bind();

private:
	GLuint ID;
};