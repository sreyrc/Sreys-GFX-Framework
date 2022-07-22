#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "ResourceManager.h"
#include "Shader.h"
#include "Mesh.h"

#include <string>

class Model
{
public:
	Model(std::string path, ResourceManager* pResourceManager);

	void Draw(Shader* shader);

private:
	std::vector<Texture*> texturesLoaded;
	std::vector<Mesh> meshes;
	std::string directory;

	void loadModel(std::string path, ResourceManager* pResourceManager);
	void processNode(aiNode* node, const aiScene* scene, ResourceManager* pResourceManager);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene, ResourceManager* pResourceManager);
	std::vector<Texture*> loadMaterialTextures(aiMaterial* mat,
		aiTextureType type, std::string typeName, ResourceManager* pResourceManager);
};

