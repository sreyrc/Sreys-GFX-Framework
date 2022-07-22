#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gDiffuse;
layout (location = 3) out vec3 gSpecular;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D myTexture;

uniform vec3 diffuse;
uniform vec3 specular;

void main() {
	gPosition = FragPos;
	gNormal = normalize(Normal);
	gDiffuse = mix(diffuse, texture(myTexture, TexCoords).rgb, 0.5f);
	gSpecular = specular;
}