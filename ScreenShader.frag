#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform float t_saturation;
uniform float t_blur;
uniform float t_outline;
uniform bool t_invert;

const float offset = 1.0 / 300.0;

void main()
{
	vec2 offsets[9] = vec2[](
		vec2(-offset, offset), // top-left
		vec2( 0.0f, offset), // top-center
		vec2( offset, offset), // top-right
		vec2(-offset, 0.0f), // center-left
		vec2( 0.0f, 0.0f), // center-center
		vec2( offset, 0.0f), // center-right
		vec2(-offset, -offset), // bottom-left
		vec2( 0.0f, -offset), // bottom-center
		vec2( offset, -offset) // bottom-right
	);

	
	float kernel_blur[9] = float[](
			t_blur/9.0f, t_blur/9.0f, t_blur/9.0f,
			t_blur/9.0f, 1 - (8 * t_blur/9.0f), t_blur/9.0f,	
			t_blur/9.0f, t_blur/9.0f, t_blur/9.0f
	);

	float kernel_outline[9] = float[](
			-t_outline, -t_outline, -t_outline,
			-t_outline, 1.0f * (1 - t_outline) + 8.0f * t_outline, -t_outline,
			-t_outline, -t_outline, -t_outline
	);

	vec3 sampleTex[9];
	for(int i = 0; i < 9; i++) {
		sampleTex[i] = texture(screenTexture, TexCoords.st + offsets[i]).rgb;
	}

	vec3 col = vec3(0.0);
	for(int i = 0; i < 9; i++)
		col += (sampleTex[i] * (kernel_blur[i] + kernel_outline[i]))/2.0f;
	vec4 color = vec4(col, 1.0);


    //vec4 color = texture(screenTexture, TexCoords);
    float average = (color.r + color.g + color.b) / 3.0;
    color.r = color.r * t_saturation + average * (1 - t_saturation);
    color.g = color.g * t_saturation + average * (1 - t_saturation);
    color.b = color.b * t_saturation + average * (1 - t_saturation);

	if (t_invert) {
		FragColor = vec4(1.0f) - color;
	}
	else {
		FragColor = color;
	}
} 