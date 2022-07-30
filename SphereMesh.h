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
        std::vector<glm::vec3> tangents;
        std::vector<glm::vec3> bitangents;


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

        //computeTangentBasis(positions, uv, normals, tangents, bitangents);

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

        for (int i = 0; i < mIndices.size() - 2; i++) {

            //int found = FindSimilarVertex(positions, uv, normals, mIndices[i + 0]);


            // Positions of vertices of triangle
            glm::vec3& v0 = positions[mIndices[i + 0]];
            glm::vec3& v1 = positions[mIndices[i + 1]];
            glm::vec3& v2 = positions[mIndices[i + 2]];

            // uv coords of triangle
            glm::vec2& uv0 = uv[mIndices[i + 0]];
            glm::vec2& uv1 = uv[mIndices[i + 1]];
            glm::vec2& uv2 = uv[mIndices[i + 2]];

            // Edges of the triangle : position delta
            glm::vec3 deltaPos1 = v1 - v0;
            glm::vec3 deltaPos2 = v2 - v0;

            // UV delta
            glm::vec2 deltaUV1 = uv1 - uv0;
            glm::vec2 deltaUV2 = uv2 - uv0;

            glm::vec3 tangent, bitangent;
            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            tangent.x = f * (deltaUV2.y * deltaPos1.x - deltaUV1.y * deltaPos2.x);
            tangent.y = f * (deltaUV2.y * deltaPos1.y - deltaUV1.y * deltaPos2.y);
            tangent.z = f * (deltaUV2.y * deltaPos1.z - deltaUV1.y * deltaPos2.z);
            bitangent.x = f * (-deltaUV2.x * deltaPos1.x + deltaUV1.x * deltaPos2.x);
            bitangent.y = f * (-deltaUV2.x * deltaPos1.y + deltaUV1.x * deltaPos2.y);
            bitangent.z = f * (-deltaUV2.x * deltaPos1.z + deltaUV1.x * deltaPos2.z);

            tangents.push_back(tangent);
            bitangents.push_back(bitangent);

            int size = tangents.size();
            int found = FindSimilarVertex(positions, uv, normals, i, size);

            if (found != -1) {
                tangents[size - 1] += tangents[found];
                bitangents[size- 1] += bitangents[found];


                //tangents.push_back(newTangent);
                //bitangents.push_back(newBitangent);
            }
            //else {
                //// Set the same tangent for all three vertices of the triangle.
                //// They will be merged later, in vboindexer.cpp

                //            tangents.push_back(tangent);
                //            tangents.push_back(tangent);

                            // Same thing for bitangents

                //            bitangents.push_back(bitangent);
                //            bitangents.push_back(bitangent);
            //}
        }

        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            mVertexData.push_back(positions[i].x);
            mVertexData.push_back(positions[i].y);
            mVertexData.push_back(positions[i].z);

            if (uv.size() > 0) {
                mVertexData.push_back(uv[i].x);
                mVertexData.push_back(uv[i].y);
            }
            if (normals.size() > 0) {
                mVertexData.push_back(normals[i].x);
                mVertexData.push_back(normals[i].y);
                mVertexData.push_back(normals[i].z);
            }
            if (i != tangents.size()) {
                mVertexData.push_back(tangents[i].x);
                mVertexData.push_back(tangents[i].y);
                mVertexData.push_back(tangents[i].z);
            }
            if (i != bitangents.size()) {
                mVertexData.push_back(bitangents[i].x);
                mVertexData.push_back(bitangents[i].y);
                mVertexData.push_back(bitangents[i].z);
            }
        }
        glBindVertexArray(mVAO);
        glBindBuffer(GL_ARRAY_BUFFER, mVBO);
        glBufferData(GL_ARRAY_BUFFER, mVertexData.size() * sizeof(float), &mVertexData[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(unsigned int), &mIndices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3 + 3 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(11 * sizeof(float)));
	}

	unsigned int GetIndexCount() {
		return static_cast<unsigned int>(mIndices.size());
	}

	void BindVAO() {
		glBindVertexArray(mVAO);
	}

private:
    int FindSimilarVertex(
        std::vector<glm::vec3>& positions,
        std::vector<glm::vec2>& uv,
        std::vector<glm::vec3>& normals,
        int index, int size) {

        for (int i = 0; i < size - 1; i++) {
            if (positions[mIndices[i]] == positions[mIndices[index]] &&
                uv[mIndices[i]] == uv[mIndices[index]] &&
                normals[mIndices[i]] == normals[mIndices[index]]) {
                return i;
            }
        }
        return -1;
    }

	GLuint mVAO, mVBO, mIBO;

	std::vector<float> mVertexData;
	std::vector<unsigned int> mIndices;

    unsigned int mIndexCount;
};