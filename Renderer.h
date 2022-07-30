#pragma once

#include "ResourceManager.h"
#include "CubeMesh.h"
#include "SphereMesh.h"
#include "QuadMesh.h"
#include "Shape.h"
#include "Cubemap.h"
#include "AudioPlayer.h"
#include "Camera.h"
#include "Model.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

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
	void SetupForDeferredShading(int SCREEN_WIDTH, int SCREEN_HEIGHT);
	void SetupFBO(const int SCREEN_WIDTH, const int SCREEN_HEIGHT);
	void SetupForHDR(const int SCREEN_WIDTH, const int SCREEN_HEIGHT);
	void SetLightVarsInShader(Shader* shader);
	void SetVertexShaderVarsForDeferredShadingAndUse(Shape* pCube, Camera* pCamera, AudioPlayer* pAudioPlayer);
	void SetShaderVarsAndUse(Shape* pSphere, Camera* pCamera, AudioPlayer* pAudioPlayer);
	void SetShapeAndDraw(Shape* pShape, bool defShadingOn, Camera* pCamera, AudioPlayer* pAudioPlayer);
	glm::mat4 CreateModelMatrix(Shape* pSphere, AudioPlayer* pAudioPlayer);
	
public:
	void AddShape(std::string name, Shape* pSphere);
	void RemoveShape(std::string name);
	void AddModel(std::string name, std::string path, ResourceManager* pResourceManager);
	std::unordered_map<std::string, Shape*>& GetShapeMap();
	
	void SetTexturePackForShape(TexturePack* texturePack, std::string name);
	void SetShapeGeometry(std::string shape, std::string name);
	std::vector<Shader*> ShapeShaderList();
	std::vector<GLuint>* GetDefShadingGBufferTextures();

public:
	// screen shader vars
	ImageFilters* mImageFilters;
	bool mSkyboxOn;

	// HDR settings
	GLfloat mExposure;
	bool mHDROn;

	// Def. Shading
	bool mDeferredShadingOn;
	
	// Clear color
	glm::vec3 mClearColor;

	GLuint mGBuffer, mAttachments[4], mGRBODepth;
	std::vector<GLuint> mGBufferTextures;

private:
	// outline properties
	glm::vec3 mSelectedShapeOutlineColor = glm::vec3(10.0f, 10.0f, 0.0f);

	float mSelectedShapeThickness = 0.03f;

	// cube storage
	std::unordered_map<std::string, Shape*> mShapeDS;

	// Model Storage
	std::unordered_map<std::string, Model*> mModelDS;
	
	// Meshes
	CubeMesh* mCubeMesh;
	SphereMesh* mSphereMesh;
	QuadMesh* mQuadMesh;

	// Shaders
	Shader* mScreenShader, *mSkyboxShader, *mOutlineShader, *mLightBlockShader, *mGBufferShader, *mGBufferShaderPBR,
		*mDeferredShadingLightingShader, *mDeferredShadingLightingShaderPBR, *mHDRShader, *mModelShader;
	
	// Proj matrix is common for all
	glm::mat4 mProj;

	Cubemap* mCubemap;

	// Skybox Mesh
	GLuint mSkyVAO, mSkyVBO;

	// framebuffer for post-processing
	GLuint mRBO, mFBO, mTextureColorBuffer;

	// FBO for HDR
	GLuint mHDRRBO, mHDRFBO, mHDRTextureColorBuffer;

	// G-Buffer framebuffer, color textures for different properties and a depth buffer
	//GLuint mGBuffer, mGPosition, mGNormal, mGDiffuseColor, mGSpecularColor, mAttachments[4], mGRBODepth;

	
	// depth framebuffer for shadow-mapping
	GLuint mShadowFBO, mShadowDepthMap;

	// Shaders that cubes can use. Each cube can decide which one to use
	std::vector<Shader*> mSphereShaders;

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

