#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

//in vec3 TangentViewPos;
//in vec3 TangentFragPos;

in mat3 TBN;

const int MAX_N_LIGHTS = 100;

struct Light {
	vec3 position;
	vec3 color;
};

uniform Light lights[MAX_N_LIGHTS];
uniform int numberOfLights;

// Material properties
uniform vec3 albedoUnif;
uniform float metalnessUnif;
uniform float roughnessUnif;
uniform float aoUnif;

// Material properties from maps
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;
uniform sampler2D depthMap;
uniform sampler2D aoMap;

// Irradiance map for IBL
uniform samplerCube irradianceMap;

uniform bool iblOn;

// Is texture pack enabled?
uniform bool packEnabled;

// Does this text have a metallic map?
uniform bool metallicMapOn;

uniform float heightScale;

uniform vec3 viewPos;

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

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir) {
	float height = texture(depthMap, texCoords).r;
	return texCoords - ((viewDir.xy / viewDir.z) * height * heightScale);
}

void main() {
	vec3 N, albedo; 
	float roughness, metalness, ao;

	if (packEnabled) {
		//vec3 TangentViewDir = normalize(TangentViewPos - TangentFragPos);
		//vec2 texCoords = ParallaxMapping(TexCoords, TangentViewDir);

		N = texture(normalMap, TexCoords).rgb;
		N  = N * 2.0f - 1.0f;
		N = normalize(TBN * N);

		albedo = texture(albedoMap, TexCoords).rgb;
		
		if (metallicMapOn) {
			roughness = texture(roughnessMap, TexCoords).r;
			metalness = texture(metallicMap, TexCoords).r;
		}
		else {
			roughness = texture(roughnessMap, TexCoords).r;
			metalness = texture(roughnessMap, TexCoords).g;
		}
		ao = texture(aoMap, TexCoords).r;
	}
	else {
		N = normalize(Normal);
		albedo = albedoUnif;
		roughness = roughnessUnif;
		metalness = metalnessUnif;
		ao = aoUnif;
	}

	//vec3 N = normalize(normal);
	//vec3 viewPos = TBN * TangentViewPos;
	vec3 V = normalize(viewPos - FragPos);
	vec3 F0 = vec3(0.4f);
	F0 = mix(F0, albedo, metalness);

	vec3 Lo = vec3(0.0f);

	for (int i = 0; i < numberOfLights; i++) {

		vec3 L = normalize(lights[i].position - FragPos);
		vec3 H  = normalize(N + L);
		float HdotV = max(dot(H, V), 0.0f); 

		float distance = length(lights[i].position - FragPos);
		float attenuation = 1.0f / (distance * distance);
		vec3 radiance = lights[i].color * attenuation;

		float D = NDFGGX(N, H, roughness);
		vec3 F  = FresnelSchlick(HdotV, F0);
		float G = SmithGGX(N, V, L, roughness);

		vec3 numerator = D * G * F; 
        float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0) + 0.0001f;// + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;
        kD *= 1.0f - metalness;	  

		float NdotL = max(dot(N, L), 0.0f);

		Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
	}

	vec3 ambient; 

	if(iblOn) {
		vec3 kS = FresnelSchlick(max(dot(N, V), 0.0), F0);
		vec3 kD = 1.0 - kS;
		kD *= 1.0 - metalness;	  
		vec3 irradiance = texture(irradianceMap, N).rgb;
		vec3 diffuse  = irradiance * albedo;
		ambient = (kD * diffuse) * ao;
	}
	else {
		ambient = vec3(0.03) * albedo * ao;
	}

	//vec3 ambient = vec3(0.03) * albedo * ao;// * texture(myTexture, TexCoords).rgb;
    vec3 color = ambient + Lo;

    FragColor = vec4(color, 1.0);
}




