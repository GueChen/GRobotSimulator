#version 420 core

layout (location = 0) in vec3 aPos;

layout(std140, set = 0, binding = 2) uniform light_sapce_parameter{
	mat4 light_space_matrices[16];
};

uniform mat4 model;

void main(){
	gl_Position = light_space_matrices[0] * model * vec4(aPos, 1.0);
}