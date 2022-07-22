#pragma once

#include <unordered_map>

#include "Texture.h"
#include "Cubemap.h"

class ResourceManager {

public:
	ResourceManager() {};

	Texture* AddTexture(std::string name, std::string path, std::string type = "texture_diffuse") {
		mTextures[name] = new Texture(path, type);
		return mTextures[name];
	}

	void AddCubeMap(std::string name, std::vector<std::string>& facePaths) {
		mCubemaps[name] = new Cubemap(facePaths);
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

private:
	std::unordered_map<std::string, Texture*> mTextures;
	std::unordered_map<std::string, Cubemap*> mCubemaps;
};