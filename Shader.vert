#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec3 Color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform float time;

void main() {
	
	float t = sin(time);

	Color.r = ((aPos.x * t + aPos.y * (1-t)) - (-0.5));
	Color.g = ((aPos.y * t + aPos.z * (1-t)) - (-0.5));
	Color.b = ((aPos.z * t + aPos.x * (1-t)) - (-0.5));

	gl_Position = proj * view * model * vec4(aPos, 1.0);
}
