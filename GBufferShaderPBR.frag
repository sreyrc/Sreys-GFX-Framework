#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gRoughMetalAO;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

in vec3 TangentViewPos;
in vec3 TangentFragPos;

in mat3 TBN;

uniform vec3 albedo;

uniform float roughness;
uniform float metalness;
uniform float ao;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;
uniform sampler2D depthMap;
uniform sampler2D aoMap;

uniform bool packEnabled;
uniform bool metallicMapOn;

uniform float heightScale;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
	float height = texture(depthMap, texCoords).r;
	return texCoords - ((viewDir.xy / viewDir.z) * height * heightScale);
}

void main() {
	gPosition = vec4(FragPos, 1.0f);

	if (packEnabled) {

		vec3 TangentViewDir = normalize(TangentViewPos - TangentFragPos);
		vec2 texCoords = ParallaxMapping(TexCoords, TangentViewDir);

		if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
			discard;

		vec3 normal = texture(normalMap, texCoords).rgb;
		normal  = normal * 2.0f - 1.0f;
		gNormal = vec4(normalize(TBN * normal), 1.0f);

		gAlbedo = vec4(texture(albedoMap, texCoords).rgb, 1.0f);
		
		if (metallicMapOn) {
			gRoughMetalAO = vec4(texture(roughnessMap, texCoords).r, texture(metallicMap, texCoords).r, texture(aoMap, texCoords).r, 1.0f);
		}
		else {
			gRoughMetalAO = vec4(texture(roughnessMap, texCoords).r, texture(roughnessMap, texCoords).g, texture(aoMap, texCoords).r, 1.0f);
		}
	}
	else {
		gNormal = vec4(normalize(Normal), 1.0f); 
		gAlbedo = vec4(albedo, 1.0f);
		gRoughMetalAO = vec4(roughness, metalness, ao, 1.0f);
	}
}