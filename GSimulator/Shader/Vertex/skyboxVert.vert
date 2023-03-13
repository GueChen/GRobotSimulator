# version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;

out vec3 tex_coords;

layout(std140, binding = 0) uniform Matrices{
    mat4 projection;
    mat4 view;
};

void main()
{
	tex_coords	= aPos;
	vec4 pos	= projection * vec4(mat3(view) * aPos, 1.0f);
	gl_Position = pos.xyww;
	
	// DEBUG usage
//	vec4 pos = projection * view * vec4(aPos, 1.0f);
//	gl_Position = pos;
	
}