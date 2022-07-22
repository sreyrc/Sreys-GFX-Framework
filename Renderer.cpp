#pragma once

#include "Renderer.h"

#include "ResourceManager.h"
#include "CubeMesh.h"
#include "SphereMesh.h"
#include "Cube.h"
#include "Cubemap.h"
#include "AudioPlayer.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <irrklang/irrKlang.h>

const int SHADOW_MAP_WIDTH = 2048;
const int SHADOW_MAP_HEIGHT = 2048;

// taken from LearnOpenGL
GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
			case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
			case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
			case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
			case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
			case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
			case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

Renderer::Renderer(const int SCREEN_WIDTH, const int SCREEN_HEIGHT, Cubemap* _cubemap) : mCubeMesh(new CubeMesh()), mSphereMesh(new SphereMesh()),
		//proj(glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, 0.1f, 100.0f)),
		mProj(glm::perspective(glm::radians(45.0f), static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT), 0.1f, 100.0f)),
		mCubemap(_cubemap), mQuadVAO(0), mQuadVBO(0), mSkyVAO(0), mSkyVBO(0), mFBO(0), mRBO(0), mTextureColorBuffer(0), mImageFilters(new ImageFilters),
		mScreenShader(new Shader("ScreenShader.vert", "ScreenShader.frag")),
		mOutlineShader(new Shader("Outlining.vert", "Outlining.frag")),
		mGBufferShader(new Shader("GBufferShader.vert", "GBufferShader.frag")),
		mGBufferShaderPBR(new Shader("GBufferShader.vert", "GBufferShaderPBR.frag")),
		mDeferredShadingLightingShader(new Shader("DeferredLightingShader.vert", "DeferredLightingShader.frag")),
		mDeferredShadingLightingShaderPBR(new Shader("DeferredLightingShader.vert", "DeferredLightingShaderPBR.frag")),
		mHDRShader(new Shader("HDR.vert", "HDR.frag")),
		mSkyboxShader(new Shader("Skybox.vert", "Skybox.frag")),
		mModelShader(new Shader("TexPhongModel.vert", "TexPhongModel.frag")),
		mSkyboxOn(true), mDeferredShadingOn(true), mHDROn(false), mExposure(1.0f)
{
	mCubeShaders.push_back(new Shader("Shader.vert", "Shader.frag"));
	mCubeShaders.push_back(new Shader("TexPhong.vert", "TexPhong.frag"));
	mCubeShaders.push_back(new Shader("TexPhong.vert", "PBR.frag"));
	mCubeShaders.push_back(new Shader("LightShader.vert", "LightShader.frag"));

	mCubeShaders[static_cast<int>(CubeType::PHONG)]->Use();
	mCubeShaders[static_cast<int>(CubeType::PHONG)]->SetInt("myTexture", 0);

	mCubeShaders[static_cast<int>(CubeType::PHONG)]->SetFloat("constant", 1.0f);
	mCubeShaders[static_cast<int>(CubeType::PHONG)]->SetFloat("linear", 0.35f);
	mCubeShaders[static_cast<int>(CubeType::PHONG)]->SetFloat("quadratic", 0.44f);

	mModelShader->SetFloat("constant", 1.0f);
	mModelShader->SetFloat("linear", 0.35f);
	mModelShader->SetFloat("quadratic", 0.44f);

	//mCubeShaders[static_cast<int>(CubeType::PBR)]->Use();
	//mCubeShaders[static_cast<int>(CubeType::PBR)]->SetInt("myTexture", 0);

	mGBufferShader->Use();
	mGBufferShader->SetInt("myTexture", 0);

	mDeferredShadingLightingShader->Use();
	mDeferredShadingLightingShader->SetInt("gPosition", 0);
	mDeferredShadingLightingShader->SetInt("gNormal", 1);
	mDeferredShadingLightingShader->SetInt("gDiffuse", 2);
	mDeferredShadingLightingShader->SetInt("gSpecular", 3);

	mDeferredShadingLightingShaderPBR->Use();
	mDeferredShadingLightingShaderPBR->SetInt("gPosition", 0);
	mDeferredShadingLightingShaderPBR->SetInt("gNormal", 1);
	mDeferredShadingLightingShaderPBR->SetInt("gAlbedo", 2);
	mDeferredShadingLightingShaderPBR->SetInt("gRoughMetalAO", 3);

	mScreenShader->Use();
	mScreenShader->SetInt("screenTexture", 0);

	mImageFilters->saturation = 1.0f;
	mImageFilters->blur = 0.0f;
	mImageFilters->outline = 0.0f;
	mImageFilters->invert = false;

	SetupSkybox();
	SetupQuadVAO();
	SetupForHDR(SCREEN_WIDTH, SCREEN_HEIGHT);
	SetupForDeferredShading(SCREEN_WIDTH, SCREEN_HEIGHT);
	SetupFBO(SCREEN_WIDTH, SCREEN_HEIGHT);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

Renderer::~Renderer() {
	delete mCubeMesh;
	delete mScreenShader;
	delete mSkyboxShader;
	delete mImageFilters;
	delete mCubemap;
	/* for (Cube* cube : mCubes) {
		delete cube;
	*/
}

void Renderer::Draw(const int SCREEN_WIDTH, const int SCREEN_HEIGHT, Camera* pCamera, AudioPlayer* pAudioPlayer) {

	glEnable(GL_DEPTH_TEST);

	//glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mHDRFBO);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// cube drawing pass

	mProj = glm::perspective(glm::radians(pCamera->mZoom),
		static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT), 0.1f, 100.0f);

	//mCubeMesh->BindVAO();
	mSphereMesh->BindVAO();

	// if doing deferred shading
	if (mDeferredShadingOn) {

		// first bind G-Buffer Framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, mGBuffer);

		// clear all color buffers and depth buffer
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// -------------- Load all geometry info of Phong-lit cubes into the FBO (multiple render targets) 
		// Now only this: Load all geometry info of PBR-lit cubes into the FBO (multiple render targets) 
		for (auto& [name, cube] : mCubeDS) {
			if (cube->mCubeType == CubeType::PBR) {
				SetVertexShaderVarsForDeferredShadingAndUse(cube, pCamera, pAudioPlayer);
				glDrawElements(GL_TRIANGLE_STRIP, mSphereMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, mHDRFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the G-Buffer textures for info on how to light the scene 
		// All drawn on a scree-sized quad
		glBindVertexArray(mQuadVAO);
		mDeferredShadingLightingShaderPBR->Use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mGPosition);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mGNormal);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mGDiffuseColor);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, mGSpecularColor);

		SetLightVarsInShader(mDeferredShadingLightingShaderPBR);
		mDeferredShadingLightingShaderPBR->SetVec3("viewPos", pCamera->mPosition);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, mGBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mHDRFBO); // write to the HDR FBO
		glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, mHDRFBO);

		// render light cubes on top of scene
		//mDeferredShadingLightingShader->SetFloat("constant", 1.0f);
		//mDeferredShadingLightingShader->SetFloat("linear", 0.35f);
		//mDeferredShadingLightingShader->SetFloat("quadratic", 0.44f);

		mSphereMesh->BindVAO();
		for (auto& [name, cube] : mCubeDS) {
			if (cube->mCubeType == CubeType::LIGHT) {
				SetShaderVarsAndUse(cube, pCamera, pAudioPlayer);
				//glDrawArrays(GL_TRIANGLES, 0, 36);
				glDrawElements(GL_TRIANGLE_STRIP, mSphereMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
			}
		}
	}
	else {
		// draw all the models
		//for (auto& [name, model] : mModelDS) {
		//	glm::mat4 modelMat = glm::mat4(1.0f);
		//	modelMat = glm::scale(modelMat, glm::vec3(10.0f));
		//	mModelShader->Use();
		//	mModelShader->SetMat4("model", modelMat);
		//	mModelShader->SetMat4("view", pCamera->GetViewMatrix());
		//	mModelShader->SetMat4("proj", mProj);
		//	mModelShader->SetVec3("viewPos", pCamera->mPosition);
		//	SetLightVarsInShader(mModelShader);
		//	model->Draw(mModelShader);
		//}

		mSphereMesh->BindVAO();
		for (const auto& [name, cube] : mCubeDS) {
			// if cube is selected - edit stencil buffer (for outlining)
			if (cube->mIsSelected) {

				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilMask(0xFF);

				SetShaderVarsAndUse(cube, pCamera, pAudioPlayer);
				glDrawElements(GL_TRIANGLE_STRIP, mSphereMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);

				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				glStencilMask(0x00);
			}
			else {
				SetShaderVarsAndUse(cube, pCamera, pAudioPlayer);
				glDrawElements(GL_TRIANGLE_STRIP, mSphereMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
			}
		}
	}
	//glDisable(GL_DEPTH_TEST); // <--- Why? Still don't understand
	mSphereMesh->BindVAO();
	// draw the outline for the selected cube
	if (!mDeferredShadingOn) {
		mOutlineShader->Use();
		mOutlineShader->SetMat4("view", pCamera->GetViewMatrix());
		mOutlineShader->SetMat4("proj", mProj);
		mOutlineShader->SetFloat("outlining", mSelectedCubeThickness);
		mOutlineShader->SetVec3("outlineColor", mSelectedCubeOutlineColor);

		for (auto& [name, cube] : mCubeDS) {
			if (cube->mIsSelected) {
				glm::mat4 model = CreateModelMatrix(cube, pAudioPlayer);
				mOutlineShader->SetMat4("model", model);
				glDrawElements(GL_TRIANGLE_STRIP, mSphereMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
				//glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
	}

	mCubeMesh->BindVAO();
	// skybox pass
	if (mSkyboxOn) {
		glDepthFunc(GL_LEQUAL);
		mSkyboxShader->Use();
		glm::mat4 view = glm::mat4(glm::mat3(pCamera->GetViewMatrix()));
		mSkyboxShader->SetMat4("view", view);
		mSkyboxShader->SetMat4("proj", mProj);
		mCubemap->Bind();
		glBindVertexArray(mSkyVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default
	}

	// tone mapping pass. Tone-map and draw it all onto another FBO which will be post-processed
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glDisable(GL_DEPTH_TEST);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mHDRShader->Use();
	mHDRShader->SetInt("hdrOn", mHDROn);
	mHDRShader->SetFloat("exposure", mExposure);
	glBindVertexArray(mQuadVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mHDRTextureColorBuffer);	// use the color attachment texture as the texture of the quad plane
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// post processing pass

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	mScreenShader->Use();
	mScreenShader->SetFloat("t_saturation", mImageFilters->saturation);
	mScreenShader->SetFloat("t_blur", mImageFilters->blur);
	mScreenShader->SetFloat("t_outline", mImageFilters->outline);
	mScreenShader->SetInt("t_invert", mImageFilters->invert);
	glBindVertexArray(mQuadVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTextureColorBuffer);	// use the color attachment texture as the texture of the quad plane
	glDrawArrays(GL_TRIANGLES, 0, 6);
}


void Renderer::SetupForShadows() {
	glGenFramebuffers(1, &mShadowFBO);

	glGenTextures(1, &mShadowFBO);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, mShadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mShadowDepthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetupSkybox() {
	glGenVertexArrays(1, &mSkyVAO);
	glGenBuffers(1, &mSkyVBO);

	glBindVertexArray(mSkyVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mSkyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mSkyboxVertices), mSkyboxVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	mSkyboxShader->SetInt("skybox", 0);
}

void Renderer::SetupQuadVAO() {
	glGenVertexArrays(1, &mQuadVAO);
	glGenBuffers(1, &mQuadVBO);

	glBindVertexArray(mQuadVAO);

	glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(sizeof(GLfloat) * 2));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void Renderer::SetupForDeferredShading(int SCREEN_WIDTH, int SCREEN_HEIGHT) {

	glGenFramebuffers(1, &mGBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mGBuffer);

	// position G-Buffer
	glGenTextures(1, &mGPosition);
	glBindTexture(GL_TEXTURE_2D, mGPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mGPosition, 0);

	// normal G-buffer
	glGenTextures(1, &mGNormal);
	glBindTexture(GL_TEXTURE_2D, mGNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mGNormal, 0);

	// diffuse color G-Buffer
	glGenTextures(1, &mGDiffuseColor);
	glBindTexture(GL_TEXTURE_2D, mGDiffuseColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mGDiffuseColor, 0);

	// specular color G-Buffer
	glGenTextures(1, &mGSpecularColor);
	glBindTexture(GL_TEXTURE_2D, mGSpecularColor);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, mGSpecularColor, 0);

	// tell OpenGL which color attachments we’ll use (of this framebuffer)
	mAttachments[0] = GL_COLOR_ATTACHMENT0;
	mAttachments[1] = GL_COLOR_ATTACHMENT1;
	mAttachments[2] = GL_COLOR_ATTACHMENT2;
	mAttachments[3] = GL_COLOR_ATTACHMENT3;
	glDrawBuffers(4, mAttachments);

	glGenRenderbuffers(1, &mGRBODepth);
	glBindRenderbuffer(GL_RENDERBUFFER, mGRBODepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mGRBODepth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer ain't complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetupFBO(const int SCREEN_WIDTH, const int SCREEN_HEIGHT) {
	glGenFramebuffers(1, &mFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

	glGenTextures(1, &mTextureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, mTextureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureColorBuffer, 0);

	glGenRenderbuffers(1, &mRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer ain't complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetupForHDR(const int SCREEN_WIDTH, const int SCREEN_HEIGHT) {

	glGenFramebuffers(1, &mHDRFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, mHDRFBO);

	glGenTextures(1, &mHDRTextureColorBuffer);
	glBindTexture(GL_TEXTURE_2D, mHDRTextureColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mHDRTextureColorBuffer, 0);

	glGenRenderbuffers(1, &mHDRRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mHDRRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mHDRRBO);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "HDR Framebuffer ain't complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetLightVarsInShader(Shader* shader) {
	int i = 0;
	for (auto& [name, cube] : mCubeDS) {
		if (cube->mCubeType == CubeType::LIGHT) {
			shader->SetVec3("lights[" + std::to_string(i) + "].position", cube->mTransform->position);
			shader->SetVec3("lights[" + std::to_string(i) + "].color", cube->mMaterial->ambient);
			i++;
		}
	}
	shader->SetInt("numberOfLights", i);
}

void Renderer::SetVertexShaderVarsForDeferredShadingAndUse(Cube* pCube, Camera* pCamera, AudioPlayer* pAudioPlayer) {
	glm::mat4 model = CreateModelMatrix(pCube, pAudioPlayer);

	if (pCube->mCubeType == CubeType::PHONG) {
		mGBufferShader->Use();
		mGBufferShader->SetMat4("model", model);
		mGBufferShader->SetMat4("view", pCamera->GetViewMatrix());
		mGBufferShader->SetMat4("proj", mProj);

		if (pCube->mTexture) {
			glActiveTexture(GL_TEXTURE0);
			pCube->mTexture->Bind();
		}
		mGBufferShader->SetVec3("diffuse", pCube->mMaterial->diffuse);
		mGBufferShader->SetVec3("specular", pCube->mMaterial->specular);
	}
	else {
		mGBufferShaderPBR->Use();
		mGBufferShaderPBR->SetMat4("model", model);
		mGBufferShaderPBR->SetMat4("view", pCamera->GetViewMatrix());
		mGBufferShaderPBR->SetMat4("proj", mProj);

		mGBufferShaderPBR->SetVec3("albedo", pCube->mMaterialPBR->albedo);
		mGBufferShaderPBR->SetFloat("roughness", pCube->mMaterialPBR->roughness);
		mGBufferShaderPBR->SetFloat("metalness", pCube->mMaterialPBR->metalness);
		mGBufferShaderPBR->SetFloat("ao", pCube->mMaterialPBR->ao);
	}
}

void Renderer::SetShaderVarsAndUse(Cube* pCube, Camera* pCamera, AudioPlayer* pAudioPlayer) {

	glm::mat4 model = CreateModelMatrix(pCube, pAudioPlayer);
	Shader* shader = mCubeShaders[static_cast<int>(pCube->mCubeType)];

	shader->Use();
	shader->SetMat4("model", model);
	shader->SetMat4("view", pCamera->GetViewMatrix());
	shader->SetMat4("proj", mProj);

	if (pCube->mCubeType == CubeType::GLOWY) {
		shader->SetFloat("time", static_cast<float>(glfwGetTime()));
	}
	else if (pCube->mCubeType == CubeType::PHONG) {
		if (pCube->mTexture) {
			glActiveTexture(GL_TEXTURE0);
			pCube->mTexture->Bind();
		}
		SetLightVarsInShader(shader);
		shader->SetVec3("material.ambient", pCube->mMaterial->ambient + glm::vec3(static_cast<float>(pAudioPlayer->GetData()) / 70000));
		shader->SetVec3("material.diffuse", pCube->mMaterial->diffuse);
		shader->SetVec3("material.specular", pCube->mMaterial->specular);
		shader->SetFloat("material.shininess", pCube->mMaterial->shininess);
		shader->SetVec3("viewPos", pCamera->mPosition);
	}
	else if (pCube->mCubeType == CubeType::PBR) {
		if (pCube->mTexture) {
			glActiveTexture(GL_TEXTURE0);
			pCube->mTexture->Bind();
		}
		SetLightVarsInShader(shader);
		shader->SetVec3("albedo", pCube->mMaterialPBR->albedo);
		shader->SetFloat("metalness", pCube->mMaterialPBR->metalness);
		shader->SetFloat("roughness", pCube->mMaterialPBR->roughness);
		shader->SetFloat("ao", pCube->mMaterialPBR->ao);
	}
	else if (pCube->mCubeType == CubeType::LIGHT) {
		shader->SetVec3("lightColor", pCube->mMaterial->ambient);
	}
}

glm::mat4 Renderer::CreateModelMatrix(Cube* pCube, AudioPlayer* pAudioPlayer) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, pCube->mTransform->position);
	glm::vec3 offset = pCube->mCubeType == CubeType::LIGHT ? 
		glm::vec3(static_cast<float>(pAudioPlayer->GetData()) / 100000) : glm::vec3(0);
	model = glm::scale(model, glm::vec3(pCube->mTransform->scale) + offset);
	model = glm::rotate(model, glm::radians(pCube->mTransform->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(pCube->mTransform->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(pCube->mTransform->rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	return model;
}

void Renderer::AddCube(std::string name, Cube* cube) {
	mCubeDS[name] = cube;
}

void Renderer::AddModel(std::string name, std::string path, ResourceManager* pResourceManager) {
	mModelDS[name] = new Model(path, pResourceManager);
}

std::unordered_map<std::string, Cube*>& Renderer::GetCubeMap() {
	return mCubeDS;
}

void Renderer::SetTextureForCube(Texture* texture, std::string name) {
	mCubeDS[name]->mTexture = texture;
}

std::vector<Shader*> Renderer::CubeShaderList() {
	return mCubeShaders;
}