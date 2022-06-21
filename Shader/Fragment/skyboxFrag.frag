# version 420 core

in vec3 TexCoords;

//uniform samplerCube cubeMap;

out vec4 FragColor;

void main()
{
	//FragColor = texture(cubeMap, TexCoords);
	FragColor = vec4(normalize(TexCoords), 1.0f);
}