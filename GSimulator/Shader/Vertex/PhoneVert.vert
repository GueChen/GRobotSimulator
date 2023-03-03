#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;

layout (std140, binding = 0) uniform Matrices{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

out vec3 FragPos;
out vec3 Norm;
out vec2 TexCoords;

void main(void)
{
    FragPos = vec3(model * vec4(aPos, 1.0f));
    Norm = transpose(inverse(mat3(model))) * aNorm;
    TexCoords = aTexCoords;
    // 转换到屏幕空间
    gl_Position = projection * view * vec4(FragPos, 1.0f);
}
