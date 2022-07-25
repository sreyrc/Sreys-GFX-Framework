#version 330 core

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gRoughMetalAO;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 albedo;

uniform float roughness;
uniform float metalness;
uniform float ao;

void main() {
	gPosition = vec4(FragPos, 1.0);
	gNormal = vec4(normalize(Normal), 1.0);
	gAlbedo = vec4(albedo, 1.0f);
	gRoughMetalAO = vec4(roughness, metalness, ao, 1.0f);
}