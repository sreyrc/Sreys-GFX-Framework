#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

const int MAX_N_LIGHTS = 100;

struct Light {
	vec3 position;
	vec3 color;
};

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gRoughMetalAO;

uniform Light lights[MAX_N_LIGHTS];
uniform int numberOfLights;

uniform vec3 viewPos;

// Position of only one light source for now (named "Light Source")
uniform vec3 lightPos;
uniform float farPlane;

uniform samplerCube shadowDepthCubeMap;

const float PI = 3.14159265359;

float NDFGGX(vec3 N, vec3 H, float alpha) {
	float NdotH = max(dot(N, H), 0.0);
	float result = (alpha * alpha)/ (PI * pow( ( (NdotH * NdotH) * ((alpha * alpha) - 1) + 1), 2));
	return result;
} 

float SchlickGGX(float NdotV, float alpha) {
	float r = (alpha + 1.0);
    float k = (r*r) / 8.0;

    return NdotV/ (NdotV * (1.0 - k) + k);
}

float SmithGGX(vec3 N, vec3 V, vec3 L, float alpha) {
	
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx1 = SchlickGGX(NdotV, alpha);  
	float ggx2 = SchlickGGX(NdotL, alpha);
	return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float ShadowCalculation(vec3 fragPos) {
	vec3 fragToLight = fragPos - lightPos;
	float closestDepth = texture(shadowDepthCubeMap, fragToLight).r;
	closestDepth *= farPlane;
	float currentDepth = length(fragToLight);
	float bias = 0.05;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	return shadow;
}

void main() {

	vec3 FragPos = texture(gPosition, TexCoords).rgb;
	vec3 N = texture(gNormal, TexCoords).rgb;
	vec3 albedo = texture(gAlbedo, TexCoords).rgb;
	float roughness = texture(gRoughMetalAO, TexCoords).r;
	float metalness = texture(gRoughMetalAO, TexCoords).g;
	float ao = texture(gRoughMetalAO, TexCoords).b;

	vec3 V = normalize(viewPos - FragPos);
	vec3 F0 = vec3(0.4);
	F0 = mix(F0, albedo, metalness);

	vec3 Lo = vec3(0);

	for (int i = 0; i < numberOfLights; i++) {

		vec3 L = normalize(lights[i].position - FragPos);
		vec3 H  = normalize(N + L);
		float HdotV = max(dot(H, V), 0.0); 

		float distance = length(lights[i].position - FragPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lights[i].color * attenuation;

		float D = NDFGGX(N, H, roughness);
		vec3 F  = FresnelSchlick(HdotV, F0);
		float G = SmithGGX(N, V, L, roughness);

		vec3 numerator = D * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metalness;	  

		float NdotL = max(dot(N, L), 0.0);

		Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
	}

	vec3 ambient = vec3(0.03) * albedo * ao;// * texture(myTexture, TexCoords).rgb;

	float shadow  = ShadowCalculation(FragPos);

    vec3 color = ambient + (1.0 - shadow) * Lo;

    FragColor = vec4(color, 1.0);
}




