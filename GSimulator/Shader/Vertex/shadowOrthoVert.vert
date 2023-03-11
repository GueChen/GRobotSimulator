#version 420 core

struct DirLight{
  vec3 dir;
  vec3 color;
};

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in vec2 aTexCoords;

layout(std140, set = 0, binding = 0) uniform Matrices{
    mat4 projection;
    mat4 view;
};
layout(std140, set = 0, binding = 2) uniform light_sapce_parameter{
	mat4 light_space_matrices[16];
};

uniform mat4 model;

out vec3 FragPos;
out vec3 Norm;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

void main()
{
    FragPos             = vec3(model * vec4(aPos, 1.0f));
    FragPosLightSpace   = light_space_matrices[0] * vec4(FragPos, 1.0f);
    Norm                = transpose(inverse(mat3(model))) * aNorm;
    TexCoords           = aTexCoords;
    gl_Position         = projection * view * vec4(FragPos, 1.0f);
}
