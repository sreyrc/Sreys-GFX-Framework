#pragma once

#include <glad/glad.h>
#include <vector>

#include <glm/glm.hpp>

class SphereMesh {

public:
	SphereMesh(): mVAO(0), mVBO(0), mIBO(0), mIndexCount(0) {

        glGenVertexArrays(1, &mVAO);

        glGenBuffers(1, &mVBO);
        glGenBuffers(1, &mIBO);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;


        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    mIndices.push_back(y * (X_SEGMENTS + 1) + x);
                    mIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    mIndices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    mIndices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }

        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            mVertexData.push_back(positions[i].x);
            mVertexData.push_back(positions[i].y);
            mVertexData.push_back(positions[i].z);
            if (uv.size() > 0)
            {
                mVertexData.push_back(uv[i].x);
                mVertexData.push_back(uv[i].y);
            }
            if (normals.size() > 0)
            {
                mVertexData.push_back(normals[i].x);
                mVertexData.push_back(normals[i].y);
                mVertexData.push_back(normals[i].z);
            }
        }
        glBindVertexArray(mVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, mVertexData.size() * sizeof(float), &mVertexData[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(unsigned int), &mIndices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
	}

	unsigned int GetIndexCount() {
		return static_cast<unsigned int>(mIndices.size());
	}

	void BindVAO() {
		glBindVertexArray(mVAO);
	}

private:
	GLuint mVAO, mVBO, mIBO;

	std::vector<float> mVertexData;
	std::vector<unsigned int> mIndices;

    unsigned int mIndexCount;
};