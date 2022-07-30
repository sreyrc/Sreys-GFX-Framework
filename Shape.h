#pragma once

#include <glm/glm.hpp>
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glfw/glfw3.h>

#include <irrklang/irrKlang.h>

#include <memory>

#include "Texture.h"
#include "AudioPlayer.h"
#include "ResourceManager.h"

enum class ShapeShading {
	GLOWY,
	PHONG,
	PBR,
	LIGHT,
	NUM
};

struct Material {
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float shininess;

	Material(glm::vec3 _ambient, glm::vec3 _diffuse, glm::vec3 _specular, float _shininess) 
		: ambient(_ambient), diffuse(_diffuse), specular(_specular), shininess(_shininess) {}
};

struct MaterialPBR {
	glm::vec3 albedo;
	float metalness;
	float roughness;
	float ao;
	TexturePack* texturePack;
	bool texturePackEnabled;

	MaterialPBR(glm::vec3 _albedo, float _metalness, float _roughness, float _ao)
		: albedo(_albedo), metalness(_metalness), roughness(_roughness), ao(_ao) {}
		//texturePack(_texturePack), texturePackEnabled(_enabled) {}
};

struct Transform {
	glm::vec3 position;
	glm::vec3 scale;
	glm::vec3 rotation;

	Transform(glm::vec3 _position, glm::vec3 _scale, glm::vec3 _rotation)
		: position(_position), scale(_scale), rotation(_rotation){}
};

class Shape
{
public:
	Shape(ResourceManager* pResourceManager, ShapeShading shading, std::string shape = "Cube") {
		
		mTransform = std::make_unique<Transform>(glm::vec3(rand() % 1, rand() % 1, rand() % 1), glm::vec3(0.5), glm::vec3(30));
		mMaterial = std::make_unique<Material>(glm::vec3(0.2f), glm::vec3(0.5f), glm::vec3(0.5f), 32.0f);
		mMaterialPBR = std::make_unique<MaterialPBR>(glm::vec3(0.2f), 0.5f, 0.5f, 0.5f);
		mShape = shape;

		mShading = shading;

		if (shading == ShapeShading::LIGHT) {
			mTransform->scale = glm::vec3(0.02f);
			mMaterial->ambient = glm::vec3(1.0f); 
		}

		mTexture = pResourceManager->GetTexture("Gravel");
		mMaterialPBR->texturePack = pResourceManager->GetTexturePack("Stylized_Fur");
		//mMaterialPBR->texturePackEnabled = true;
	}

	~Shape() {}

	Texture* mTexture;
	std::unique_ptr<Transform>(mTransform);
	std::unique_ptr<Material>(mMaterial);
	std::unique_ptr<MaterialPBR>(mMaterialPBR);

	ShapeShading mShading;
	std::string mName;
	std::string mShape;

	bool mIsSelected = false;
};

