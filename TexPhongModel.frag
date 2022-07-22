#version 330 core

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

const int MAX_N_LIGHTS = 100;

struct Light {
	vec3 position;
	vec3 color;
};

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

uniform float constant;
uniform float linear;
uniform float quadratic;

uniform Material material;
uniform Light lights[MAX_N_LIGHTS];

uniform vec3 viewPos;

uniform int numberOfLights;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

out vec4 FragColor;

void main() {
	vec3 result = vec3(0);
	for(int i = 0; i < numberOfLights; i++) {
		vec3 ambient = vec3(0.1f) * lights[i].color;

		vec3 norm = normalize(Normal);

		vec3 lightDir = lights[i].position - FragPos;
		float diff = max(dot(norm, lightDir), 0.0f);
		vec3 diffuse = diff * texture(texture_diffuse1, TexCoords).rgb * lights[i].color;

		vec3 viewDir = viewPos - FragPos;
		vec3 halfwayDir = normalize(viewDir + lightDir);
		float spec = pow(max(dot(halfwayDir, norm), 0.0f), 64.0f);
		vec3 specular = spec * texture(texture_specular1, TexCoords).rgb * lights[i].color;

		float dist = length(lights[i].position - FragPos);
		float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;

		result += (ambient + diffuse + specular); 

	}
	FragColor = vec4(result, 1.0f);
}
