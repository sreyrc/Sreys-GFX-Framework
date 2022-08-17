#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_glfw.h"

#include "Renderer.h"
#include "ResourceManager.h"
#include "AudioPlayer.h"
#include "Camera.h"

class Editor
{
public:
	Editor(GLFWwindow* window) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330");

		// UI coloring
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Header] = ImColor(200, 100, 0);
		style.Colors[ImGuiCol_HeaderHovered] = ImColor(200, 100, 0);
		style.Colors[ImGuiCol_HeaderActive] = ImColor(200, 200, 0);

		style.Colors[ImGuiCol_SliderGrab] = ImColor(200, 200, 0);
		style.Colors[ImGuiCol_SliderGrabActive] = ImColor(300, 200, 0);

		style.Colors[ImGuiCol_FrameBg] = ImColor(100, 50, 0);
		style.Colors[ImGuiCol_FrameBgHovered] = ImColor(200, 100, 0);

		style.Colors[ImGuiCol_TitleBg] = ImColor(100, 50, 0);
		style.Colors[ImGuiCol_TitleBgActive] = ImColor(200, 100, 0);


		// configure type map
		mShapeShadingMap["Phong"] = ShapeShading::PHONG;
		mShapeShadingMap["PBR"] = ShapeShading::PBR;
		mShapeShadingMap["Glowy"] = ShapeShading::GLOWY;
		mShapeShadingMap["Light"] = ShapeShading::LIGHT;

		mShapeGeometryList.push_back("Cube");
		mShapeGeometryList.push_back("Sphere");
		mShapeGeometryList.push_back("Quad");
	}

	~Editor() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Update(Renderer* pRenderer, ResourceManager* pResourceManager, AudioPlayer* pAudioHandler, Camera* pCamera) {

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		auto& shapeMap = pRenderer->GetShapeMap();
		auto& textureMap = pResourceManager->GetTextureList();
		auto& texturePackMap = pResourceManager->GetTexturePackList();

		// List of cubes in the scene
		ImGui::Begin("Shape List");
		{
			if (ImGui::BeginListBox("##Shapes", ImVec2(200.0f, 100.0f)));
			for (auto& [name, shape] : shapeMap)
			{
				const bool is_selected = (mSelectedShape == name);
				if (ImGui::Selectable(name.c_str(), is_selected)) {
					mSelectedShape = name;
					for (auto& [name, c] : shapeMap) {
						c->mIsSelected = false;
					}
					shape->mIsSelected = true;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}
		if (ImGui::Button("Add Shape")) {
			pRenderer->AddShape("Shape " + std::to_string(++mShapeCount), new Shape(pResourceManager, ShapeShading::PBR));
		}

		if (ImGui::Button("Add 10 Shapes")) {
			for (int i = 0; i < 10; i++) {
				pRenderer->AddShape("Shape " + std::to_string(++mShapeCount), new Shape(pResourceManager, ShapeShading::PBR));
			}
		}

		ImGui::End();

		// Cube properties
		ImGui::Begin("Shape Properties"); {
			if (ImGui::BeginListBox("Type", ImVec2(200.0f, 100.0f))) {
				for (auto& [name, type] : mShapeShadingMap)
				{
					if (pRenderer->mDeferredShadingOn && (type == ShapeShading::GLOWY || type == ShapeShading::PHONG)) {
						continue;
					}
					else {
						const bool is_selected = (mSelectedShapeShading == name);
						if (ImGui::Selectable(name.c_str(), is_selected)) {
							mSelectedShapeShading = name;
							if (mSelectedShapeShading == "Light") {
								shapeMap[mSelectedShape]->mTransform->scale = glm::vec3(0.02f, 0.02f, 0.02f);
							}
							else {
								shapeMap[mSelectedShape]->mTransform->scale = glm::vec3(0.5f, 0.5f, 0.5f);
							}
							shapeMap[mSelectedShape]->mShading = mShapeShadingMap[mSelectedShapeShading];
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndListBox();
			}

			if (ImGui::BeginListBox("Geometry", ImVec2(200.0f, 100.0f))) {
				for (int i = 0; i < mShapeGeometryList.size(); i++) {
					const bool is_selected = (mSelectedGeometry == i);
					if (ImGui::Selectable(mShapeGeometryList[i].c_str(), is_selected)) {
						mSelectedGeometry = i;
						pRenderer->SetShapeGeometry(mShapeGeometryList[i], mSelectedShape);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}

			if (shapeMap[mSelectedShape]->mShading == ShapeShading::PBR) {
				ImGui::Checkbox("Enable Tex Pack", &shapeMap[mSelectedShape]->mMaterialPBR->texturePackEnabled);
			}

			if (shapeMap[mSelectedShape]->mMaterialPBR->texturePackEnabled) {
				if (shapeMap[mSelectedShape]->mShading == ShapeShading::PBR) {
					if (ImGui::BeginListBox("Texture Packs", ImVec2(200.0f, 100.0f))); {
						for (auto& [name, texPack] : texturePackMap)
						{
							if (name == "") {
								continue;
							}
							const bool is_selected = (mSelectedTexturePack == name);
							if (ImGui::Selectable(name.c_str(), is_selected)) {
								mSelectedTexturePack = name;
								pRenderer->SetTexturePackForShape(pResourceManager->GetTexturePack(mSelectedTexturePack), mSelectedShape);
							}

							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndListBox();
					}
				}
			}

			ImGui::Text("Transform");

			ImGui::SliderFloat3("Position", &(shapeMap[mSelectedShape]->mTransform.get()->position.x), -2, 2);

			if (!(shapeMap[mSelectedShape]->mShading == ShapeShading::LIGHT)) {
				ImGui::SliderFloat3("Scale", &(shapeMap[mSelectedShape]->mTransform.get()->scale.x), -2, 2);
				ImGui::SliderFloat3("Rotation", &(shapeMap[mSelectedShape]->mTransform.get()->rotation.x), 0, 180);
			}			

			if (shapeMap[mSelectedShape]->mShading == ShapeShading::PHONG) {
				ImGui::Text("Material Properties");

				ImGui::SliderFloat3("Ambient", &shapeMap[mSelectedShape]->mMaterial.get()->ambient.r, 0, 10);
				ImGui::ColorEdit3("Diffuse", &shapeMap[mSelectedShape]->mMaterial.get()->diffuse.r);
				ImGui::ColorEdit3("Specular", &shapeMap[mSelectedShape]->mMaterial.get()->specular.r);

				ImGui::SliderFloat("Shininess", &(shapeMap[mSelectedShape]->mMaterial.get()->shininess), 0, 128);
			}

			else if (shapeMap[mSelectedShape]->mShading == ShapeShading::PBR) {
				ImGui::Text("Material Properties");

				ImGui::ColorEdit3("Albedo", &shapeMap[mSelectedShape]->mMaterialPBR->albedo.r);
				ImGui::SliderFloat("Metalness", &shapeMap[mSelectedShape]->mMaterialPBR->metalness, 0, 1);
				ImGui::SliderFloat("Roughness", &shapeMap[mSelectedShape]->mMaterialPBR->roughness, 0, 1);
				ImGui::SliderFloat("AO", &shapeMap[mSelectedShape]->mMaterialPBR->ao, 0, 1);
			}

			else {
				ImGui::SliderFloat3("Ambient", &shapeMap[mSelectedShape]->mMaterial.get()->ambient.r, 0, 10);
			}

			if (ImGui::Button("Remove") && mShapeCount > 1) {
				pRenderer->RemoveShape(mSelectedShape); mShapeCount--;
				mSelectedShape = (shapeMap.begin())->first;
			}
		}
		
		ImGui::End();

		ImGui::Begin("Filters"); {
			ImGui::SliderFloat("Saturation", &pRenderer->mImageFilters->saturation, 0, 1);
			ImGui::SliderFloat("Blur", &pRenderer->mImageFilters->blur, 0, 1);
			ImGui::SliderFloat("Outline", &pRenderer->mImageFilters->outline, 0, 1);
			ImGui::Checkbox("Invert", &pRenderer->mImageFilters->invert);
		}
		ImGui::End();

		ImGui::Begin("Diffuse IBL"); {
			ImGui::Checkbox("IBL", &pRenderer->mSkyboxOn);
			// Show list of different env. maps to choose from
			//if (pRenderer->mSkyboxOn) {

			//}
		}
		ImGui::End();

		// Audio Player
		ImGui::Begin("Audio Player");
		{
			auto trackList = pAudioHandler->GetSongs();

			if (ImGui::BeginListBox("##Audio"));
			{
				for (auto& [name, path] : trackList)
				{
					const bool is_selected = (mSelectedTrack == name);
					if (ImGui::Selectable(name.c_str(), is_selected)) {
						mSelectedTrack = name;
						pAudioHandler->SetCurrentSong(mSelectedTrack);
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();

			if (ImGui::Button("Play")) {
				pAudioHandler->Play();
			}

			if (ImGui::Button("Pause")) {
				pAudioHandler->Pause();
			}
		}
		ImGui::End();

		ImGui::Begin("Camera"); {
			ImGui::SliderFloat3("Position", &pCamera->mPosition.x, -10.0f, 10.0f);
			ImGui::Text("Angle");
			ImGui::SliderFloat("Pitch", &pCamera->mPitch, -90, 90);
			ImGui::SliderFloat("Yaw", &pCamera->mYaw, 0, 360);
			ImGui::SliderFloat("Zoom", &pCamera->mZoom, 30, 120);
		}
		ImGui::End();

		ImGui::Begin("Other Options"); {
			ImGui::Checkbox("HDR", &pRenderer->mHDROn);
			ImGui::SliderFloat("Exposure", &pRenderer->mExposure, 0, 5);
			ImGui::Checkbox("Deferred Shading", &pRenderer->mDeferredShadingOn);
			ImGui::ColorEdit3("BG Color", &pRenderer->mClearColor.r);
		}
		ImGui::End();

		if (pRenderer->mDeferredShadingOn) {
			ImGui::Begin("G-Buffers (Def. Shading)"); {
				int vecSize = pRenderer->mGBufferTextures.size();
				for (int i = 0; i < vecSize; i++) {
					ImGui::Image((ImTextureID)pRenderer->mGBufferTextures[i], ImVec2(384, 216), ImVec2(0, 1), ImVec2(1, 0));
				}
			}
			ImGui::End();
		}

		ImGui::ShowMetricsWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

private:
	std::unordered_map <std::string, ShapeShading> mShapeShadingMap;
	std::vector<std::string> mShapeGeometryList;
	std::string mSelectedTexturePack = "Leather_Padded", mSelectedTrack,
		mSelectedShape = "PBR Shape", mSelectedShapeShading = "Textured";
	int mSelectedGeometry = 0;
	int mShapeCount = 2;
};
