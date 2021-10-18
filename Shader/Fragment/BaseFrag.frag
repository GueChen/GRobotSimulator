#version 420 core

uniform bool colorReverse;
out vec4 gl_FragColor;

void main(void)
{
    if(colorReverse)
        gl_FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f); // 黑色着色器
    else
        gl_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); // 白色着色器
}
