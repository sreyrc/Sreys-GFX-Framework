#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gRoughMetalAO;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 albedo;

uniform float roughness;
uniform float metalness;
uniform float ao;

void main() {
	gPosition = FragPos;
	gNormal = normalize(Normal);
	gAlbedo = albedo;
	gRoughMetalAO = vec3(roughness, metalness, ao);
}