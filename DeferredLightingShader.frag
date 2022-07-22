#version 330 core

in vec2 TexCoords;

const int MAX_N_LIGHTS = 100;

struct Light {
	vec3 position;
	vec3 color;
};

uniform Light lights[MAX_N_LIGHTS];

uniform vec3 viewPos;

uniform int numberOfLights;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;

uniform float constant;
uniform float linear;
uniform float quadratic;

out vec4 FragColor;

void main() {
	vec3 result = vec3(0);

	vec3 FragPos = texture(gPosition, TexCoords).rgb;
	vec3 Normal = texture(gNormal, TexCoords).rgb;
	vec3 diffuse = texture(gDiffuse, TexCoords).rgb;
	vec3 specular = texture(gSpecular, TexCoords).rgb;
	
	for(int i = 0; i < numberOfLights; i++) {
		//vec3 ambient = material.ambient * lights[i].color;
		vec3 ambient = vec3(0.1f) * diffuse * lights[i].color;

		vec3 norm = normalize(Normal);

		vec3 lightDir = lights[i].position - FragPos;
		float diff = max(dot(norm, lightDir), 0.0f);
		vec3 diffuse = diff * diffuse * lights[i].color;
		//vec3 diffuse = diff * texture(myTexture, TexCoords).rgb * lightColor;

		vec3 viewDir = viewPos - FragPos;
		vec3 halfwayDir = normalize(viewDir + lightDir);
		float spec = pow(max(dot(halfwayDir, norm), 0.0f), 64.0f);
		vec3 specular = spec * specular * lights[i].color;

		float dist = length(lights[i].position - FragPos);
		float attenuation = 1.0 / (constant + linear * dist + quadratic * (dist * dist));

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;

		result += (ambient + diffuse + specular); 
	}
	FragColor = vec4(result, 1.0f);
}
