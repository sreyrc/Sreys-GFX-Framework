#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform float outlining;

void main()
{
	gl_Position = proj * view * model * vec4(aPos + aNormal * outlining, 1.0);
}