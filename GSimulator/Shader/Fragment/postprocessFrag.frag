#version 420 core

in vec2 TexCoords;

layout(binding = 8)uniform sampler2D screen_texture;

out vec4 FragColor;

void main()
{	
	FragColor = vec4(vec3(texture(screen_texture, TexCoords)), 1.0f);
}