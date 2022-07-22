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
		mCubeTypeMap["Phong"] = CubeType::PHONG;
		mCubeTypeMap["PBR"] = CubeType::PBR;
		mCubeTypeMap["Glowy"] = CubeType::GLOWY;
		mCubeTypeMap["Light"] = CubeType::LIGHT;
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

		auto& cubeDS = pRenderer->GetCubeMap();
		auto& textureMap = pResourceManager->GetTextureList();

		// List of cubes in the scene
		ImGui::Begin("Cube List");
		{
			if (ImGui::BeginListBox("##Cubes", ImVec2(200.0f, 100.0f)));
			for (auto& [name, cube] : cubeDS)
			{
				const bool is_selected = (mSelectedCube == name);
				if (ImGui::Selectable(name.c_str(), is_selected)) {
					mSelectedCube = name;
					for (auto& [name, c] : cubeDS) {
						c->mIsSelected = false;
					}
					cube->mIsSelected = true;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}
		if (ImGui::Button("+")) {
			pRenderer->AddCube("Cube " + std::to_string(++mCubeCount), new Cube(pResourceManager, CubeType::PHONG));
		}
		ImGui::End();

		ImGui::Begin("Cube Properties");
		{
			if (ImGui::BeginListBox("Type", ImVec2(200.0f, 100.0f)));
			for (auto& [name, type] : mCubeTypeMap)
			{
				const bool is_selected = (mSelectedCubeType == name);
				if (ImGui::Selectable(name.c_str(), is_selected)) {
					mSelectedCubeType = name;
					cubeDS[mSelectedCube]->mCubeType = mCubeTypeMap[mSelectedCubeType];
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();

			if (cubeDS[mSelectedCube]->mCubeType == CubeType::PHONG) {
				if (ImGui::BeginListBox("Textures", ImVec2(200.0f, 100.0f)));
				for (auto& [name, tex] : textureMap)
				{
					const bool is_selected = (mSelectedTexture == name);
					if (ImGui::Selectable(name.c_str(), is_selected)) {
						mSelectedTexture = name;
						pRenderer->SetTextureForCube(pResourceManager->GetTexture(mSelectedTexture), mSelectedCube);
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}

			ImGui::Text("Transform");

			ImGui::SliderFloat3("Position", &(cubeDS[mSelectedCube]->mTransform.get()->position.x), -2, 2);
			ImGui::SliderFloat3("Scale", &(cubeDS[mSelectedCube]->mTransform.get()->scale.x), -2, 2);
			ImGui::SliderFloat3("Rotation", &(cubeDS[mSelectedCube]->mTransform.get()->rotation.x), 0, 180);

			if (cubeDS[mSelectedCube]->mCubeType == CubeType::PHONG) {
				ImGui::Text("Material Properties");

				ImGui::SliderFloat3("Ambient", &cubeDS[mSelectedCube]->mMaterial.get()->ambient.r, 0, 10);
				ImGui::ColorEdit3("Diffuse", &cubeDS[mSelectedCube]->mMaterial.get()->diffuse.r);
				ImGui::ColorEdit3("Specular", &cubeDS[mSelectedCube]->mMaterial.get()->specular.r);

				ImGui::SliderFloat("Shininess", &(cubeDS[mSelectedCube]->mMaterial.get()->shininess), 0, 128);
			}

			else if (cubeDS[mSelectedCube]->mCubeType == CubeType::PBR) {
				ImGui::Text("Material Properties");

				ImGui::ColorEdit3("Albedo", &cubeDS[mSelectedCube]->mMaterialPBR->albedo.r);
				ImGui::SliderFloat("Metalness", &cubeDS[mSelectedCube]->mMaterialPBR->metalness, 0, 1);
				ImGui::SliderFloat("Roughness", &cubeDS[mSelectedCube]->mMaterialPBR->roughness, 0, 1);
				ImGui::SliderFloat("AO", &cubeDS[mSelectedCube]->mMaterialPBR->ao, 0, 1);
			}

			else {
				ImGui::SliderFloat3("Ambient", &cubeDS[mSelectedCube]->mMaterial.get()->ambient.r, 0, 10);
			}
		}
		
		ImGui::End();

		ImGui::Begin("Filters");
		{
			ImGui::SliderFloat("Saturation", &pRenderer->mImageFilters->saturation, 0, 1);
			ImGui::SliderFloat("Blur", &pRenderer->mImageFilters->blur, 0, 1);
			ImGui::SliderFloat("Outline", &pRenderer->mImageFilters->outline, 0, 1);
			ImGui::Checkbox("Invert", &pRenderer->mImageFilters->invert);
			ImGui::Checkbox("Skybox", &pRenderer->mSkyboxOn);
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

		ImGui::Begin("HDR"); {
			ImGui::Checkbox("HDR", &pRenderer->mHDROn);
			ImGui::SliderFloat("Exposure", &pRenderer->mExposure, 0, 5);
		}
		ImGui::End();

		ImGui::Begin("Deferred Shading"); {
			ImGui::Checkbox("On", &pRenderer->mDeferredShadingOn);
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

private:
	std::unordered_map <std::string, CubeType> mCubeTypeMap;
	std::string mSelectedTexture, mSelectedTrack, mSelectedCube = "PBR Cube", mSelectedCubeType = "Textured";
	int mCubeCount = 1;
};
