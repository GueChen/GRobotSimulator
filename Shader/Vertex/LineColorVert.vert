#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aColor;

layout(std140, binding = 0) uniform Matrices{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

out vec3 vertColor;

void main(void)
{
    vertColor   = aColor;
    gl_Position = projection *
                  view *
                  model * vec4(aPos, 1.0f);
}
