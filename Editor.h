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
		mSphereTypeMap["Phong"] = SphereType::PHONG;
		mSphereTypeMap["PBR"] = SphereType::PBR;
		mSphereTypeMap["Glowy"] = SphereType::GLOWY;
		mSphereTypeMap["Light"] = SphereType::LIGHT;
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

		auto& sphereMap = pRenderer->GetSphereMap();
		auto& textureMap = pResourceManager->GetTextureList();

		// List of cubes in the scene
		ImGui::Begin("Sphere List");
		{
			if (ImGui::BeginListBox("##Spheres", ImVec2(200.0f, 100.0f)));
			for (auto& [name, sphere] : sphereMap)
			{
				const bool is_selected = (mSelectedSphere == name);
				if (ImGui::Selectable(name.c_str(), is_selected)) {
					mSelectedSphere = name;
					for (auto& [name, c] : sphereMap) {
						c->mIsSelected = false;
					}
					sphere->mIsSelected = true;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndListBox();
		}
		if (ImGui::Button("+")) {
			pRenderer->AddSphere("Sphere " + std::to_string(++mSphereCount), new Sphere(pResourceManager, SphereType::PHONG));
		}
		ImGui::End();

		// Cube properties
		ImGui::Begin("Sphere Properties");
		{

			if (ImGui::BeginListBox("Type", ImVec2(200.0f, 100.0f)));
			for (auto& [name, type] : mSphereTypeMap)
			{
				if (pRenderer->mDeferredShadingOn && (type == SphereType::GLOWY || type == SphereType::PHONG)) {
					continue;
				}
				else {
					const bool is_selected = (mSelectedSphereType == name);
					if (ImGui::Selectable(name.c_str(), is_selected)) {
						mSelectedSphereType = name;
						sphereMap[mSelectedSphere]->mType = mSphereTypeMap[mSelectedSphereType];
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();


			if (sphereMap[mSelectedSphere]->mType == SphereType::PHONG) {
				if (ImGui::BeginListBox("Textures", ImVec2(200.0f, 100.0f)));
				for (auto& [name, tex] : textureMap)
				{
					const bool is_selected = (mSelectedTexture == name);
					if (ImGui::Selectable(name.c_str(), is_selected)) {
						mSelectedTexture = name;
						pRenderer->SetTextureForSphere(pResourceManager->GetTexture(mSelectedTexture), mSelectedSphere);
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}

			ImGui::Text("Transform");

			ImGui::SliderFloat3("Position", &(sphereMap[mSelectedSphere]->mTransform.get()->position.x), -2, 2);
			ImGui::SliderFloat3("Scale", &(sphereMap[mSelectedSphere]->mTransform.get()->scale.x), -2, 2);
			ImGui::SliderFloat3("Rotation", &(sphereMap[mSelectedSphere]->mTransform.get()->rotation.x), 0, 180);

			if (sphereMap[mSelectedSphere]->mType == SphereType::PHONG) {
				ImGui::Text("Material Properties");

				ImGui::SliderFloat3("Ambient", &sphereMap[mSelectedSphere]->mMaterial.get()->ambient.r, 0, 10);
				ImGui::ColorEdit3("Diffuse", &sphereMap[mSelectedSphere]->mMaterial.get()->diffuse.r);
				ImGui::ColorEdit3("Specular", &sphereMap[mSelectedSphere]->mMaterial.get()->specular.r);

				ImGui::SliderFloat("Shininess", &(sphereMap[mSelectedSphere]->mMaterial.get()->shininess), 0, 128);
			}

			else if (sphereMap[mSelectedSphere]->mType == SphereType::PBR) {
				ImGui::Text("Material Properties");

				ImGui::ColorEdit3("Albedo", &sphereMap[mSelectedSphere]->mMaterialPBR->albedo.r);
				ImGui::SliderFloat("Metalness", &sphereMap[mSelectedSphere]->mMaterialPBR->metalness, 0, 1);
				ImGui::SliderFloat("Roughness", &sphereMap[mSelectedSphere]->mMaterialPBR->roughness, 0, 1);
				ImGui::SliderFloat("AO", &sphereMap[mSelectedSphere]->mMaterialPBR->ao, 0, 1);
			}

			else {
				ImGui::SliderFloat3("Ambient", &sphereMap[mSelectedSphere]->mMaterial.get()->ambient.r, 0, 10);
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
					ImGui::Image((ImTextureID)pRenderer->mGBufferTextures[i], ImVec2(192, 108), ImVec2(0, 1), ImVec2(1, 0));
				}
			}
			ImGui::End();
		}

		ImGui::ShowMetricsWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

private:
	std::unordered_map <std::string, SphereType> mSphereTypeMap;
	std::string mSelectedTexture, mSelectedTrack, mSelectedSphere = "PBR Sphere", mSelectedSphereType = "Textured";
	int mSphereCount = 1;
};
