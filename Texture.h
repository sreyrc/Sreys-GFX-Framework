#pragma once

#include <glad/glad.h>
#include <string>

class Texture {

public:
    Texture(std::string path, std::string& type);
    void Bind();

    std::string GetType();
    std::string GetPath();

private:
    GLuint mID;
    std::string mType, mPath;
};