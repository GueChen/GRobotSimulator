#version 420 core

uniform bool colorReverse;
out vec4 frag_color;

void main(void)
{
    if(colorReverse)
        frag_color = vec4(0.0f, 0.0f, 0.0f, 1.0f); // 黑色着色器
    else
        frag_color = vec4(1.0f, 1.0f, 1.0f, 1.0f); // 白色着色器
}
