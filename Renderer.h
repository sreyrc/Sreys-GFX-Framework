#pragma once

#include "ResourceManager.h"
#include "CubeMesh.h"
#include "SphereMesh.h"
#include "Cube.h"
#include "Cubemap.h"
#include "AudioPlayer.h"
#include "Camera.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

class Renderer
{
public:
	struct ImageFilters {
		float saturation;
		float blur;
		float outline;
		bool invert;
	};

	Renderer(const int SCREEN_WIDTH, const int SCREEN_HEIGHT, Cubemap* _cubemap);
	~Renderer();

	void Draw(const int SCREEN_WIDTH, const int SCREEN_HEIGHT, Camera* pCamera, AudioPlayer* pAudioPlayer);

private:
	// Setup Stuff
	void SetupForShadows();
	void SetupSkybox();
	void SetupQuadVAO();
	void SetupForDeferredShading(int SCREEN_WIDTH, int SCREEN_HEIGHT);
	void SetupFBO(const int SCREEN_WIDTH, const int SCREEN_HEIGHT);
	void SetupForHDR(const int SCREEN_WIDTH, const int SCREEN_HEIGHT);
	void SetLightVarsInShader(Shader* shader);
	void SetVertexShaderVarsForDeferredShadingAndUse(Cube* pCube, Camera* pCamera, AudioPlayer* pAudioPlayer);
	void SetShaderVarsAndUse(Cube* pCube, Camera* pCamera, AudioPlayer* pAudioPlayer);
	glm::mat4 CreateModelMatrix(Cube* pCube, AudioPlayer* pAudioPlayer);
	
public:
	void AddCube(std::string name, Cube* cube);
	void AddModel(std::string name, std::string path, ResourceManager* pResourceManager);
	std::unordered_map<std::string, Cube*>& GetCubeMap();
	
	void SetTextureForCube(Texture* texture, std::string name);
	std::vector<Shader*> CubeShaderList();

public:
	// screen shader vars
	ImageFilters* mImageFilters;
	bool mSkyboxOn;

	// HDR settings
	GLfloat mExposure;
	bool mHDROn;

	// Def. Shading
	bool mDeferredShadingOn;

private:
	// outline properties
	glm::vec3 mSelectedCubeOutlineColor = glm::vec3(10.0, 10.0, 0.0);
	float mSelectedCubeThickness = 0.03f;

	// cube storage
	std::unordered_map<std::string, Cube*> mCubeDS;

	// Model Storage
	std::unordered_map<std::string, Model*> mModelDS;
	CubeMesh* mCubeMesh;
	SphereMesh* mSphereMesh;

	// Shaders
	Shader* mScreenShader, *mSkyboxShader, *mOutlineShader, *mLightBlockShader, *mGBufferShader, *mGBufferShaderPBR,
		*mDeferredShadingLightingShader, *mDeferredShadingLightingShaderPBR, *mHDRShader, *mModelShader;
	
	// Proj matrix is common for all
	glm::mat4 mProj;

	Cubemap* mCubemap;

	// Quad mesh
	GLuint mQuadVAO, mQuadVBO;

	// Skybox Mesh
	GLuint mSkyVAO, mSkyVBO;

	// framebuffer for post-processing
	GLuint mRBO, mFBO, mTextureColorBuffer;

	// FBO for HDR
	GLuint mHDRRBO, mHDRFBO, mHDRTextureColorBuffer;

	// G-Buffer framebuffer, color textures for different properties and a depth buffer
	GLuint mGBuffer, mGPosition, mGNormal, mGDiffuseColor, mGSpecularColor, mAttachments[4], mGRBODepth;
	
	// depth framebuffer for shadow-mapping
	GLuint mShadowFBO, mShadowDepthMap;

	// Shaders that cubes can use. Each cube can decide which one to use
	std::vector<Shader*> mCubeShaders;

	// vertex attributes for a quad that fills the entire screen in NDC
	GLfloat quadVertices[24] = { 
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
	};

	GLfloat mSkyboxVertices[108] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
};

