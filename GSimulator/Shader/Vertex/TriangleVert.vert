#version 420 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 aFragColor;
uniform mat4 model;
void main(void)
{
    gl_Position = model * vec4(aPos, 1.0f);
    aFragColor = aColor;
}
