# version 420 core

in vec3 tex_coords;

uniform samplerCube cubemap_texture;

out vec4 FragColor;

#define PI 3.1415926535

void main()
{
	FragColor = texture(cubemap_texture, vec3(tex_coords.y, tex_coords.z, -tex_coords.x));
//	float r		= length(tex_coords);
//	float theta = (tex_coords.z / r + PI / 2.0f) / PI;
//	
//	FragColor = vec4(mix(vec3(0.05f), vec3(0.25f, 0.25f, 0.25f), theta), 1.0f);

	//FragColor = vec4(normalize(TexCoords), 1.0f);
//	if(TexCoords.z > 0.0f)
//		FragColor = vec4(mix(vec3(0.35f, 0.35f, 0.45f), vec3(0.55f, 0.65f, 1.0f), tex_coords.z), 1.0f);
//	else
//		FragColor = vec4(mix(vec3(0.05f, 0.05f, 0.05f), vec3(0.35f, 0.35f, 0.45f), (1.0f + tex_coords.z)), 1.0f);
}