# version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;

out vec3 TexCoords;

layout(std140, binding = 0) uniform Matrices{
    mat4 projection;
    mat4 view;
};

void main()
{
	TexCoords	= aPos;
	vec4 pos	= projection * vec4(mat3(view) * aPos, 1.0f);
	gl_Position = pos.xyww;
}