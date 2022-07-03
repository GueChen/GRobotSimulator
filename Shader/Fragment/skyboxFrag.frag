# version 420 core

in vec3 TexCoords;

//uniform samplerCube cubeMap;

out vec4 FragColor;

void main()
{
	//FragColor = texture(cubeMap, TexCoords);
	//FragColor = vec4(normalize(TexCoords), 1.0f);
	if(TexCoords.z > 0.6f)
		FragColor = vec4(mix(vec3(0.90f, 0.90f, 1.0f),vec3(0.35f, 0.35f, 0.55f), TexCoords.z), 1.0f);
	else
		FragColor = vec4(mix(vec3(0.1f, 0.1f, 0.1f),vec3(0.35f, 0.35f, 0.35f), (1.0f + TexCoords.z)), 1.0f);
}