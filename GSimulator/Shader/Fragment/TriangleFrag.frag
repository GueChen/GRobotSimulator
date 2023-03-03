#version 420 core

in vec3 aFragColor;

out vec4 FragColor;

void main(void)
{
    FragColor = vec4(aFragColor, 1.0f);
}
