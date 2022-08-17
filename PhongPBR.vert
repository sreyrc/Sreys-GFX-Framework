#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

//out vec3 TangentFragPos;
//out vec3 TangentViewPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

//uniform vec3 viewPos;

out mat3 TBN;

void main() {
	gl_Position = proj * view * model * vec4(aPos, 1.0);
	FragPos = vec3(model * vec4(aPos, 1.0f));
	TexCoords = aTexCoords;
	Normal = transpose(inverse(mat3(model))) * aNormal;
	vec3 T = normalize(transpose(inverse(mat3(model))) * aTangent);
	vec3 B = normalize(transpose(inverse(mat3(model))) * aBitangent);
	vec3 N = normalize(Normal);
	TBN = mat3(T, B, N);
//	mat3 invTBN = transpose(TBN);
//	TangentFragPos = invTBN * FragPos;
//	TangentViewPos = invTBN * viewPos;
}
