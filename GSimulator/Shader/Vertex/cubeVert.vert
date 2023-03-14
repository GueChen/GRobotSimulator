#version 450 core

layout (location = 0) in vec3 apos;

out vec3 tex_coords;

layout(std140, binding = 0) uniform Matrices{
    mat4 projection;
    mat4 view;
};

void main(){
    tex_coords  = apos;
    gl_Position = projection * view * vec4(apos, 1.0f);
}