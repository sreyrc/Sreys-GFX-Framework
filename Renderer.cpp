#pragma once

#include "Renderer.h"

#include "ResourceManager.h"
#include "CubeMesh.h"
#include "SphereMesh.h"
#include "Shape.h"
#include "Cubemap.h"
#include "AudioPlayer.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <irrklang/irrKlang.h>


const int SHADOW_MAP_WIDTH = 2048;
const int SHADOW_MAP_HEIGHT = 2048;

// error checking code - taken from LearnOpenGL
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

Renderer::Renderer(const int SCREEN_WIDTH, const int SCREEN_HEIGHT, Cubemap* _cubemap, ResourceManager* pResourceManager) :
	mCubeMesh(new CubeMesh()), mSphereMesh(new SphereMesh()), mQuadMesh(new QuadMesh()),
	//proj(glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, 0.1f, 100.0f)),
	mProj(glm::perspective(glm::radians(45.0f), static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT), 0.1f, 100.0f)),
	mCubemap(_cubemap), mSkyVAO(0), mSkyVBO(0), mFBO(0), mRBO(0), mTextureColorBuffer(0), mImageFilters(new ImageFilters),
	mScreenShader(new Shader("ScreenShader.vert", "ScreenShader.frag")),
	mOutlineShader(new Shader("Outlining.vert", "Outlining.frag")),
	mGBufferShader(new Shader("GBufferShader.vert", "GBufferShader.frag")),
	mGBufferShaderPBR(new Shader("GBufferShader.vert", "GBufferShaderPBR.frag")),
	mDeferredShadingLightingShader(new Shader("DeferredLightingShader.vert", "DeferredLightingShader.frag")),
	mDeferredShadingLightingShaderPBR(new Shader("DeferredLightingShader.vert", "DeferredLightingShaderPBR.frag")),
	mHDRShader(new Shader("HDR.vert", "HDR.frag")),
	mSkyboxShader(new Shader("Skybox.vert", "Skybox.frag")),
	mModelShader(new Shader("PhongModel.vert", "PhongModel.frag")),
	mPointShadowDepthShader(new Shader("PointShadowDepth.vert", "PointShadowDepth.frag", "PointShadowDepth.geom")),
	mEquiRecToCubeMapShader(new Shader("EquiRecToCubemap.vert", "EquiRecToCubemap.frag")),
	mSkyboxOn(true), mDeferredShadingOn(true), mHDROn(false), mExposure(1.0f), mClearColor(glm::vec3(0)), mGBufferTextures(4),
	mShadowTransforms(6), mShadowProj(glm::perspective(glm::radians(90.0f), static_cast<float>(1024.0f)/1024.0f, 1.0f, 25.0f)),
	mCaptureProj(glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f)), mCaptureViews(6)
{
	mShapeShaders.push_back(new Shader("Shader.vert", "Shader.frag"));
	mShapeShaders.push_back(new Shader("PhongPBR.vert", "Phong.frag"));
	mShapeShaders.push_back(new Shader("PhongPBR.vert", "PBR.frag"));
	mShapeShaders.push_back(new Shader("LightShader.vert", "LightShader.frag"));

	mShapeShaders[static_cast<int>(ShapeShading::PHONG)]->Use();
	mShapeShaders[static_cast<int>(ShapeShading::PHONG)]->SetInt("myTexture", 0);

	mShapeShaders[static_cast<int>(ShapeShading::PHONG)]->SetFloat("constant", 1.0f);
	mShapeShaders[static_cast<int>(ShapeShading::PHONG)]->SetFloat("linear", 0.35f);
	mShapeShaders[static_cast<int>(ShapeShading::PHONG)]->SetFloat("quadratic", 0.44f);

	mShapeShaders[static_cast<int>(ShapeShading::PBR)]->Use();
	mShapeShaders[static_cast<int>(ShapeShading::PBR)]->SetInt("albedoMap", 0);
	mShapeShaders[static_cast<int>(ShapeShading::PBR)]->SetInt("normalMap", 1);
	mShapeShaders[static_cast<int>(ShapeShading::PBR)]->SetInt("roughnessMap", 2);
	mShapeShaders[static_cast<int>(ShapeShading::PBR)]->SetInt("metallicMap", 3);
	mShapeShaders[static_cast<int>(ShapeShading::PBR)]->SetInt("depthMap", 4);
	mShapeShaders[static_cast<int>(ShapeShading::PBR)]->SetInt("aoMap", 5);
	mShapeShaders[static_cast<int>(ShapeShading::PBR)]->SetInt("irradianceMap", 6);

	//mModelShader->SetFloat("constant", 1.0f);
	//mModelShader->SetFloat("linear", 0.35f);
	//mModelShader->SetFloat("quadratic", 0.44f);

	//mCubeShaders[static_cast<int>(CubeType::PBR)]->Use();
	//mCubeShaders[static_cast<int>(CubeType::PBR)]->SetInt("myTexture", 0);

	mGBufferShader->Use();
	mGBufferShader->SetInt("myTexture", 0);

	mGBufferShaderPBR->Use();
	mGBufferShaderPBR->SetInt("albedoMap", 0);
	mGBufferShaderPBR->SetInt("normalMap", 1);
	mGBufferShaderPBR->SetInt("roughnessMap", 2);
	mGBufferShaderPBR->SetInt("metallicMap", 3);
	mGBufferShaderPBR->SetInt("depthMap", 4);
	mGBufferShaderPBR->SetInt("aoMap", 5);

	//mDeferredShadingLightingShader->Use();
	//mDeferredShadingLightingShader->SetInt("gPosition", 0);
	//mDeferredShadingLightingShader->SetInt("gNormal", 1);
	//mDeferredShadingLightingShader->SetInt("gDiffuse", 2);
	//mDeferredShadingLightingShader->SetInt("gSpecular", 3);
	mSkyboxShader->SetInt("skybox", 0);

	mDeferredShadingLightingShaderPBR->Use();
	mDeferredShadingLightingShaderPBR->SetInt("gPosition", 0);
	mDeferredShadingLightingShaderPBR->SetInt("gNormal", 1);
	mDeferredShadingLightingShaderPBR->SetInt("gAlbedo", 2);
	mDeferredShadingLightingShaderPBR->SetInt("gRoughMetalAO", 3);
	mDeferredShadingLightingShaderPBR->SetInt("shadowDepthCubeMap", 4);

	mScreenShader->Use();
	mScreenShader->SetInt("screenTexture", 0);

	//mCaptureViews[0] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//mCaptureViews[1] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//mCaptureViews[2] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//mCaptureViews[3] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	//mCaptureViews[4] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//mCaptureViews[5] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	mImageFilters->saturation = 1.0f;
	mImageFilters->blur = 0.0f;
	mImageFilters->outline = 0.0f;
	mImageFilters->invert = false;

	SetupSkybox();
	SetupForIBL(pResourceManager);
	SetupForShadows();
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
}

void Renderer::Draw(const int SCREEN_WIDTH, const int SCREEN_HEIGHT, Camera* pCamera, AudioPlayer* pAudioPlayer) {

	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, mHDRFBO);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(mClearColor.r, mClearColor.y, mClearColor.z, 1.0f);

	
	// Shape Drawing Pass
	mProj = glm::perspective(glm::radians(pCamera->mZoom),
		static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT), 0.1f, 100.0f);


	// if doing deferred shading
	// Only for PBR. (And light cubes obviously)
	if (mDeferredShadingOn) {

		// --------- SHADOW PASS ---------

		// using only one point light source right now
		glm::vec3 lightPos = mShapeDS["Light Source"]->mTransform->position;

		// Setting up shadow transforms for each face of the Cubemap
		mShadowTransforms[0] = mShadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
		mShadowTransforms[1] = mShadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
		mShadowTransforms[2] = mShadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
		mShadowTransforms[3] = mShadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
		mShadowTransforms[4] = mShadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
		mShadowTransforms[5] = mShadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));

		// Draw to cubemap depth texture to create shadow map
		glViewport(0, 0, 1024, 1024);
		glBindFramebuffer(GL_FRAMEBUFFER, mShadowDepthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		mPointShadowDepthShader->Use();
		for (unsigned int i = 0; i < 6; ++i)
			mPointShadowDepthShader->SetMat4("shadowMatrices[" + std::to_string(i) + "]", mShadowTransforms[i]);
		mPointShadowDepthShader->SetFloat("farPlane", 25.0f);
		mPointShadowDepthShader->SetVec3("lightPos", lightPos);
		for (auto& [name, shape] : mShapeDS) {
			if (shape->mShading != ShapeShading::LIGHT) {
				glm::mat4 model = CreateModelMatrix(shape, pAudioPlayer);
				mPointShadowDepthShader->SetMat4("model", model);
				SetShapeAndDraw(shape);
			}
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// --------- GEOMETRY PASS ---------

		// First bind G-Buffer Framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, mGBuffer);

		// Clear all color buffers and depth buffer
		glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(mClearColor.r, mClearColor.y, mClearColor.z, 1.0f);

		// Load all geometry info of PBR-lit spheres into the FBO (multiple render targets) 
		for (auto& [name, shape] : mShapeDS) {
			if (shape->mShading == ShapeShading::PBR) {
				SetVertexShaderVarsForDeferredShadingAndUse(shape, pCamera, pAudioPlayer);
					//SetShapeAndDraw(shape, mDeferredShadingOn, pCamera, pAudioPlayer);
						//if (pShape->mShading == ShapeShading::PBR) {
				SetShapeAndDraw(shape);
			}
		}

		// ------ LIGHTING/COLOR PASS ------ 

		glBindFramebuffer(GL_FRAMEBUFFER, mHDRFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the G-Buffer textures for info on how to light the scene 
		// All drawn on a screen-sized quad
		mQuadMesh->BindVAO();
		mDeferredShadingLightingShaderPBR->Use();


		// Bind all G-Buffer textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mGBufferTextures[0]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mGBufferTextures[1]);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mGBufferTextures[2]);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, mGBufferTextures[3]);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mShadowDepthCubeMap);

		// Set up shader vars
		SetLightVarsInShader(mDeferredShadingLightingShaderPBR);
		mDeferredShadingLightingShaderPBR->SetVec3("viewPos", pCamera->mPosition);
		mDeferredShadingLightingShaderPBR->SetVec3("lightPos", lightPos);
		mDeferredShadingLightingShaderPBR->SetFloat("farPlane", 25.0f);

		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, mGBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mHDRFBO); // write to the HDR FBO
		glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, mHDRFBO);


		//mDeferredShadingLightingShader->SetFloat("constant", 1.0f);
		//mDeferredShadingLightingShader->SetFloat("linear", 0.35f);
		//mDeferredShadingLightingShader->SetFloat("quadratic", 0.44f);

		// Render light spheres on top of scene;
		for (auto& [name, shape] : mShapeDS) {
			if (shape->mShading == ShapeShading::LIGHT) {
				SetShaderVarsAndUse(shape, pCamera, pAudioPlayer);
				SetShapeAndDraw(shape);
			}
		}
	}
	else {
		// Draw all the models
		//for (auto& [name, model] : mModelDS) {
		//	glm::mat4 modelMat = glm::mat4(1.0f);
		//	modelMat = glm::scale(modelMat, glm::vec3(1.0f));
		//	modelMat = glm::translate(modelMat, glm::vec3(0.1f));
		//	modelMat = glm::rotate(modelMat, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		//	modelMat = glm::rotate(modelMat, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//	modelMat = glm::rotate(modelMat, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//	mModelShader->Use();
		//	mModelShader->SetMat4("model", modelMat);
		//	mModelShader->SetMat4("view", pCamera->GetViewMatrix());
		//	mModelShader->SetMat4("proj", mProj);
		//	mModelShader->SetVec3("viewPos", pCamera->mPosition);
		//	SetLightVarsInShader(mModelShader);
		//	model->Draw(mModelShader);
		//}

		//mSphereMesh->BindVAO();
		for (const auto& [name, shape] : mShapeDS) {
			// if shape is selected - edit stencil buffer (for outlining)
			if (shape->mIsSelected) {

				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glStencilMask(0xFF);

				SetShaderVarsAndUse(shape, pCamera, pAudioPlayer);
				SetShapeAndDraw(shape);

				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				glStencilMask(0x00);
			}
			else {
				SetShaderVarsAndUse(shape, pCamera, pAudioPlayer);
				SetShapeAndDraw(shape);
			}
		}
	}
	//glDisable(GL_DEPTH_TEST); // <--- Why? Still don't understand
	//mSphereMesh->BindVAO();
	// draw the outline for the selected cube
	if (!mDeferredShadingOn) {
		mOutlineShader->Use();
		mOutlineShader->SetMat4("view", pCamera->GetViewMatrix());
		mOutlineShader->SetMat4("proj", mProj);
		mOutlineShader->SetFloat("outlining", mSelectedShapeThickness);
		mOutlineShader->SetVec3("outlineColor", mSelectedShapeOutlineColor);

		for (auto& [name, shape] : mShapeDS) {
			if (shape->mIsSelected) {
				glm::mat4 model = CreateModelMatrix(shape, pAudioPlayer);
				mOutlineShader->SetMat4("model", model);
				SetShapeAndDraw(shape);
			}
		}

		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
	}

	//mCubeMesh->BindVAO();
	
	
	// ------ Draw BG ------

	if (mSkyboxOn) {
		glDepthFunc(GL_LEQUAL);
		mEquiRecToCubeMapShader->Use();
		glm::mat4 view = glm::mat4(glm::mat3(pCamera->GetViewMatrix()));
		mEquiRecToCubeMapShader->SetMat4("view", view);
		mEquiRecToCubeMapShader->SetMat4("proj", mProj);
		glActiveTexture(GL_TEXTURE0);
		mHDRIBLTextureBG->Bind();
		glBindVertexArray(mSkyVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
	}
	
	//if (mSkyboxOn) {
	//	glDepthFunc(GL_LEQUAL);
	//	mSkyboxShader->Use();
	//	glm::mat4 view = glm::mat4(glm::mat3(pCamera->GetViewMatrix()));
	//	mSkyboxShader->SetMat4("view", view);
	//	mSkyboxShader->SetMat4("proj", mProj);
	//	mCubemap->Bind();
	//	glBindVertexArray(mSkyVAO);
	//	glDrawArrays(GL_TRIANGLES, 0, 36);
	//	glBindVertexArray(0);
	//	glDepthFunc(GL_LESS); // set depth function back to default
	//}

	// Tone-mapping pass. Tone-map and draw it all onto another FBO which will be post-processed
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	glDisable(GL_DEPTH_TEST);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mHDRShader->Use();
	mHDRShader->SetInt("hdrOn", mHDROn);
	mHDRShader->SetFloat("exposure", mExposure);
	mQuadMesh->BindVAO();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mHDRTextureColorBuffer);	// use the color attachment texture as the texture of the quad plane
	glDrawArrays(GL_TRIANGLES, 0, 6);

	
	// ------ POST-PROCESSING PASS ------
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	mScreenShader->Use();
	mScreenShader->SetFloat("t_saturation", mImageFilters->saturation);
	mScreenShader->SetFloat("t_blur", mImageFilters->blur);
	mScreenShader->SetFloat("t_outline", mImageFilters->outline);
	mScreenShader->SetInt("t_invert", mImageFilters->invert);
	mQuadMesh->BindVAO();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTextureColorBuffer);	// use the color attachment texture as the texture of the quad plane
	glDrawArrays(GL_TRIANGLES, 0, 6);
}


void Renderer::SetupForShadows() {
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	
	glGenFramebuffers(1, &mShadowDepthMapFBO);

	// Create depth cubemap texture
	glGenTextures(1, &mShadowDepthCubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mShadowDepthCubeMap);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// Attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, mShadowDepthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mShadowDepthCubeMap, 0);
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

void Renderer::SetupForIBL(ResourceManager* pResourceManager) {
	glGenFramebuffers(1, &mCaptureFBO);
	glGenRenderbuffers(1, &mCaptureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, mCaptureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mCaptureRBO);

	glGenTextures(1, &mEnvCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, mEnvCubemap);
	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	mCaptureViews[0] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	mCaptureViews[1] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	mCaptureViews[2] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	mCaptureViews[3] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	mCaptureViews[4] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	mCaptureViews[5] = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));


	// ------ IBL PASS: HDR TO CUBEMAP CONVERSION ------

	mHDRIBLTextureBG = pResourceManager->GetHDRImage("Ditch_River", 0);
	mHDRIBLTextureIrrMap = pResourceManager->GetHDRImage("Ditch_River", 1);

	mEquiRecToCubeMapShader->Use();
	mEquiRecToCubeMapShader->SetInt("equirectangularMap", 0);
	mEquiRecToCubeMapShader->SetMat4("projection", mCaptureProj);

	glActiveTexture(GL_TEXTURE0);
	mHDRIBLTextureIrrMap->Bind();

	glViewport(0, 0, 512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, mCaptureFBO);

	for (unsigned int i = 0; i < 6; ++i)
	{
		mEquiRecToCubeMapShader->SetMat4("view", mCaptureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mEnvCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(mSkyVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::SetupForDeferredShading(int SCREEN_WIDTH, int SCREEN_HEIGHT) {

	glGenFramebuffers(1, &mGBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, mGBuffer);

	// Position G-Buffer
	glGenTextures(1, &mGBufferTextures[0]);
	glBindTexture(GL_TEXTURE_2D, mGBufferTextures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mGBufferTextures[0], 0);

	// Normal G-buffer
	glGenTextures(1, &mGBufferTextures[1]);
	glBindTexture(GL_TEXTURE_2D, mGBufferTextures[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mGBufferTextures[1], 0);

	// Diffuse color or Albedo G-Buffer
	glGenTextures(1, &mGBufferTextures[2]);
	glBindTexture(GL_TEXTURE_2D, mGBufferTextures[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mGBufferTextures[2], 0);

	// Roughness, metalness and ambient occlusion values buffer
	glGenTextures(1, &mGBufferTextures[3]);
	glBindTexture(GL_TEXTURE_2D, mGBufferTextures[3]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, mGBufferTextures[3], 0);

	// Tell OpenGL which color attachments we’ll use (of this framebuffer)
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
	for (auto& [name, cube] : mShapeDS) {
		if (cube->mShading == ShapeShading::LIGHT) {
			shader->SetVec3("lights[" + std::to_string(i) + "].position", cube->mTransform->position);
			shader->SetVec3("lights[" + std::to_string(i) + "].color", cube->mMaterial->ambient);
			i++;
		}
	}
	shader->SetInt("numberOfLights", i);
}

void Renderer::SetVertexShaderVarsForDeferredShadingAndUse(Shape* pShape, Camera* pCamera, AudioPlayer* pAudioPlayer) {

	glm::mat4 model = CreateModelMatrix(pShape, pAudioPlayer);
	mGBufferShaderPBR->Use();
	mGBufferShaderPBR->SetMat4("model", model);
	mGBufferShaderPBR->SetMat4("view", pCamera->GetViewMatrix());
	mGBufferShaderPBR->SetMat4("proj", mProj);
	mGBufferShaderPBR->SetInt("packEnabled", pShape->mMaterialPBR->texturePackEnabled);

	if (pShape->mMaterialPBR->texturePackEnabled) {
		glActiveTexture(GL_TEXTURE0);
		if (pShape->mMaterialPBR->texturePack->albedoMap) {
			pShape->mMaterialPBR->texturePack->albedoMap->Bind();
		}

		glActiveTexture(GL_TEXTURE1);
		if (pShape->mMaterialPBR->texturePack->normalMap) {
			pShape->mMaterialPBR->texturePack->normalMap->Bind();
		}

		glActiveTexture(GL_TEXTURE2);
		if (pShape->mMaterialPBR->texturePack->roughnessMap) {
			pShape->mMaterialPBR->texturePack->roughnessMap->Bind();
		}

		glActiveTexture(GL_TEXTURE3);
		if (pShape->mMaterialPBR->texturePack->metallicMap) {
			pShape->mMaterialPBR->texturePack->metallicMap->Bind();
			mGBufferShaderPBR->SetInt("metallicMapOn", true);
		}
		else {
			mGBufferShaderPBR->SetInt("metallicMapOn", false);
		}

		glActiveTexture(GL_TEXTURE4);
		if (pShape->mMaterialPBR->texturePack->depthMap) {
			pShape->mMaterialPBR->texturePack->depthMap->Bind();
			mGBufferShaderPBR->SetVec3("viewPos", pCamera->mPosition);
		}

		glActiveTexture(GL_TEXTURE5);
		if (pShape->mMaterialPBR->texturePack->aoMap) {
			pShape->mMaterialPBR->texturePack->aoMap->Bind();
		}

		mGBufferShaderPBR->SetFloat("heightScale", 0.1f);
	}
	else {
		mGBufferShaderPBR->SetVec3("albedo", pShape->mMaterialPBR->albedo);
		mGBufferShaderPBR->SetFloat("roughness", pShape->mMaterialPBR->roughness);
		mGBufferShaderPBR->SetFloat("metalness", pShape->mMaterialPBR->metalness);
		mGBufferShaderPBR->SetFloat("ao", pShape->mMaterialPBR->ao);
	}
}

void Renderer::SetShaderVarsAndUse(Shape* pShape, Camera* pCamera, AudioPlayer* pAudioPlayer) {

	glm::mat4 model = CreateModelMatrix(pShape, pAudioPlayer);
	Shader* shader = mShapeShaders[static_cast<int>(pShape->mShading)];

	shader->Use();
	shader->SetMat4("model", model);
	shader->SetMat4("view", pCamera->GetViewMatrix());
	shader->SetMat4("proj", mProj);

	if (pShape->mShading == ShapeShading::GLOWY) {
		shader->SetFloat("time", static_cast<float>(glfwGetTime()));
	}
	else if (pShape->mShading == ShapeShading::PHONG) {
		SetLightVarsInShader(shader);
		shader->SetVec3("material.ambient", pShape->mMaterial->ambient + glm::vec3(static_cast<float>(pAudioPlayer->GetData()) / 70000));
		shader->SetVec3("material.diffuse", pShape->mMaterial->diffuse);
		shader->SetVec3("material.specular", pShape->mMaterial->specular);
		shader->SetFloat("material.shininess", pShape->mMaterial->shininess);
		shader->SetVec3("viewPos", pCamera->mPosition);
	}
	else if (pShape->mShading == ShapeShading::PBR) {
		shader->SetVec3("viewPos", pCamera->mPosition);
		SetLightVarsInShader(shader);
		shader->SetInt("packEnabled", pShape->mMaterialPBR->texturePackEnabled);
		if (pShape->mMaterialPBR->texturePackEnabled) {
			glActiveTexture(GL_TEXTURE0);
			if (pShape->mMaterialPBR->texturePack->albedoMap) {
				pShape->mMaterialPBR->texturePack->albedoMap->Bind();
			}

			glActiveTexture(GL_TEXTURE1);
			if (pShape->mMaterialPBR->texturePack->normalMap) {
				pShape->mMaterialPBR->texturePack->normalMap->Bind();
			}

			glActiveTexture(GL_TEXTURE2);
			if (pShape->mMaterialPBR->texturePack->roughnessMap) {
				pShape->mMaterialPBR->texturePack->roughnessMap->Bind();
			}

			glActiveTexture(GL_TEXTURE3);
			if (pShape->mMaterialPBR->texturePack->metallicMap) {
				pShape->mMaterialPBR->texturePack->metallicMap->Bind();
				shader->SetInt("metallicMapOn", true);
			}
			else {
				shader->SetInt("metallicMapOn", false);
			}

			glActiveTexture(GL_TEXTURE4);
			if (pShape->mMaterialPBR->texturePack->depthMap) {
				pShape->mMaterialPBR->texturePack->depthMap->Bind();
			}

			glActiveTexture(GL_TEXTURE5);
			if (pShape->mMaterialPBR->texturePack->aoMap) {
				pShape->mMaterialPBR->texturePack->aoMap->Bind();
			}
			
			shader->SetFloat("heightScale", 0.1f);
		}
		else {
			shader->SetVec3("albedoUnif", pShape->mMaterialPBR->albedo);
			shader->SetFloat("roughnessUnif", pShape->mMaterialPBR->roughness);
			shader->SetFloat("metalnessUnif", pShape->mMaterialPBR->metalness);
			shader->SetFloat("aoUnif", pShape->mMaterialPBR->ao);
		}
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mEnvCubemap);
	}
	else if (pShape->mShading == ShapeShading::LIGHT) {
		shader->SetVec3("lightColor", pShape->mMaterial->ambient);
	}
}

void Renderer::SetShapeAndDraw(Shape* pShape) {

	if (pShape->mShape == "Sphere") {
		mSphereMesh->BindVAO();
		glDrawElements(GL_TRIANGLE_STRIP, mSphereMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
	}
	if (pShape->mShape == "Cube") {
		mCubeMesh->BindVAO();
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	if (pShape->mShape == "Quad") {
		mQuadMesh->BindVAO();
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}

glm::mat4 Renderer::CreateModelMatrix(Shape* pShape, AudioPlayer* pAudioPlayer) {
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, pShape->mTransform->position);
	model = glm::scale(model, glm::vec3(pShape->mTransform->scale) + glm::vec3(static_cast<float>(pAudioPlayer->GetData()) / 100000));

	model = glm::rotate(model, glm::radians(pShape->mTransform->rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(pShape->mTransform->rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(pShape->mTransform->rotation.z), glm::vec3(1.0f, 0.0f, 1.0f));

	return model;
}

void Renderer::AddShape(std::string name, Shape* pShape) {
	mShapeDS[name] = pShape;
}

void Renderer::RemoveShape(std::string name) {
	mShapeDS.erase(name);
}

void Renderer::AddModel(std::string name, std::string path, ResourceManager* pResourceManager) {
	mModelDS[name] = new Model(path, pResourceManager);
}

std::unordered_map<std::string, Shape*>& Renderer::GetShapeMap() {
	return mShapeDS;
}

void Renderer::SetTexturePackForShape(TexturePack* texturePack, std::string name) {
	mShapeDS[name]->mMaterialPBR->texturePack = texturePack;
}

void Renderer::SetShapeGeometry(std::string shape, std::string name) {
	mShapeDS[name]->mShape = shape;
}

std::vector<Shader*> Renderer::ShapeShaderList() {
	return mShapeShaders;
}

std::vector<GLuint>* Renderer::GetDefShadingGBufferTextures() {
	return &mGBufferTextures;
}
