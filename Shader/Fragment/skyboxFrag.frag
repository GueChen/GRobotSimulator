# version 420 core

in vec3 TexCoords;

uniform samplerCube cubemap_texture;

out vec4 FragColor;

void main()
{
	FragColor = texture(cubemap_texture, vec3(TexCoords.y, TexCoords.z, -TexCoords.x));
	//FragColor = vec4(normalize(TexCoords), 1.0f);
//	if(TexCoords.z > 0.0f)
//		FragColor = vec4(mix(vec3(0.35f, 0.35f, 0.45f), vec3(0.55f, 0.65f, 1.0f), TexCoords.z), 1.0f);
//	else
//		FragColor = vec4(mix(vec3(0.05f, 0.05f, 0.05f), vec3(0.35f, 0.35f, 0.45f), (1.0f + TexCoords.z)), 1.0f);
}