#include "Model.h"

Model::Model(std::string path, ResourceManager* pResourceManager) {
	loadModel(path, pResourceManager);
}

void Model::Draw(Shader* shader)
{
	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(shader);
	}
}

void Model::loadModel(std::string path, ResourceManager* pResourceManager)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate |
		aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP" << importer.GetErrorString() << '\n';
		return;
	}

	directory = path.substr(0, path.find_last_of('/'));
	processNode(scene->mRootNode, scene, pResourceManager);

}

void Model::processNode(aiNode* node, const aiScene* scene, ResourceManager* pResourceManager)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene, pResourceManager));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene, pResourceManager);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene, ResourceManager* pResourceManager) {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture*> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;

		vertex.position = glm::vec3(mesh->mVertices[i].x, 
									mesh->mVertices[i].y, 
									mesh->mVertices[i].z);

		if (mesh->HasNormals()) {
			vertex.normal = glm::vec3(mesh->mNormals[i].x,
									  mesh->mNormals[i].y,
									  mesh->mNormals[i].z);
		}

		if (mesh->mTextureCoords[0]) {
			vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y);

			//vertex.tangent = glm::vec3(mesh->mTangents[i].x,
			//						   mesh->mTangents[i].y,
			//						   mesh->mTangents[i].z);

			//vertex.bitangent = glm::vec3(mesh->mBitangents[i].x,
			//						     mesh->mBitangents[i].y,
			//						     mesh->mBitangents[i].z);
		}
		else {
			vertex.texCoords = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);

		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];

			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		std::vector<Texture*> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", pResourceManager);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		
		std::vector<Texture*> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", pResourceManager);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		
		std::vector<Texture*> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", pResourceManager);
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
		
		std::vector<Texture*> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", pResourceManager);
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		return Mesh(vertices, indices, textures);
	}
}

std::vector<Texture*> Model::loadMaterialTextures(aiMaterial* mat,
						aiTextureType type, std::string typeName, 
						ResourceManager* pResourceManager) {

	std::vector<Texture*> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		/*std::string path(str.C_Str());*/
		mat->GetTexture(type, i, &str);
		// cannot remain as "../"backpack" - must be changed 
		std::string path = "../resources/objects/backpack/" + std::string(str.C_Str());
		bool skip = false;
		for (unsigned int j = 0; j < texturesLoaded.size(); j++) {
			if (std::strcmp(texturesLoaded[j]->GetPath().c_str(), path.c_str()) == 0) {
				textures.push_back(texturesLoaded[j]);
				skip = true; 
				break;
			}
		}
		if (!skip) {
			Texture* texture = pResourceManager->AddTexture("Texxx " + typeName + ' ' + std::to_string(i), path, typeName);
			textures.push_back(texture);
			texturesLoaded.push_back(texture);
		}
	}

	return textures;
}



