#pragma once

#include <unordered_map>

#include "Texture.h"
#include "Cubemap.h"

#include <fstream>

struct TexturePack {
	Texture* albedoMap;
	Texture* normalMap;
	Texture* roughnessMap;
	Texture* metallicMap;
	Texture* aoMap;

	TexturePack(Texture* _albedoMap, Texture* _normalMap, Texture* _roughnessMap, Texture* _metallicMap, Texture* _aoMap)
		: albedoMap(_albedoMap), normalMap(_normalMap), roughnessMap(_roughnessMap), metallicMap(_metallicMap), aoMap(_aoMap) {}
};

class ResourceManager {

public:
	ResourceManager() {

		stbi_set_flip_vertically_on_load(true);

		std::ifstream input;
		input.open("../resources/texture_packs/Texture_Pack_List.txt");

		if (!input.is_open()) {
			std::cout << "Ahh shit" << '\n';
		}

		while (input) {
			std::string texPackName;
			std::getline(input, texPackName, '\n');

			std::vector<std::string> paths(5);
			std::string basePathName = "../resources/texture_packs/" + texPackName + "/" + texPackName;
			paths[0] = basePathName + "_BaseColor.jpg";
			paths[1] = basePathName + "_Normal.jpg";
			paths[2] = basePathName + "_Roughness.jpg";
			paths[3] = basePathName + "_Metallic.jpg";
			paths[4] = basePathName + "_AO.jpg";

			AddTexturePack(texPackName, paths);
		}
	};

	Texture* AddTexture(std::string name, std::string path, std::string type = "texture_diffuse") {
		std::ifstream input;
		input.open(path);
		if (!input.is_open()) {
			return nullptr;
		}
		else {
			mTextures[name] = new Texture(path, type);
			return mTextures[name];
		}
	}

	TexturePack* AddTexturePack(std::string name, std::vector<std::string>& paths) {		
		mTexturePacks[name] = new TexturePack(
			AddTexture(name + "_Albedo", paths[0]),
			AddTexture(name + "_Normal", paths[1]),
			AddTexture(name + "_Roughness", paths[2]),
			AddTexture(name + "_Metallness", paths[3]),
			AddTexture(name + "_AO", paths[4]));

		return mTexturePacks[name];
	}

	void AddCubeMap(std::string name, std::vector<std::string>& facePaths) {
		mCubemaps[name] = new Cubemap(facePaths);
	}

	TexturePack* GetTexturePack(std::string name) {
		if (mTexturePacks.find(name) != mTexturePacks.end()) {
			return mTexturePacks[name];
		}
		else {
			return nullptr;
		}
	}

	Texture* GetTexture(std::string name) {
		if (mTextures.find(name) != mTextures.end()) {
			return mTextures[name];
		}
		else {
			return nullptr;
		}
	}

	Cubemap* GetCubeMap(std::string name) {
		if (mCubemaps.find(name) != mCubemaps.end()) {
			return mCubemaps[name];
		}
		else {
			return nullptr;
		}
	}

	std::unordered_map<std::string, Texture*>& GetTextureList() {
		return mTextures;
	}

	std::unordered_map<std::string, TexturePack*>& GetTexturePackList() {
		return mTexturePacks;
	}

private:
	std::unordered_map<std::string, Texture*> mTextures;
	std::unordered_map<std::string, TexturePack*> mTexturePacks;
	std::unordered_map<std::string, Cubemap*> mCubemaps;
};