#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;

layout (std140, binding = 0) uniform Matrices{
    mat4 projection;
    mat4 view;
};

uniform bool normal_reverse = false;
uniform mat4 model;

out vec3 world_pos;
out vec3 normal;
out vec2 tex_coords;

void main(void)
{
    world_pos    = vec3(model * vec4(aPos, 1.0f));
    normal      = transpose(inverse(mat3(model))) * aNorm;
    if(normal_reverse) normal = -normal;
    tex_coords  = aTexCoords;    
    gl_Position = projection * view * vec4(world_pos, 1.0f);
}
