#version 450 core

layout(location = 0) in vec3 apos;
layout(location = 1) in vec3 anorm;
layout(location = 2) in vec2 atexcoords;

out vec2 tex_coords;

void main(){
	tex_coords = atexcoords;
	gl_Position = vec4(apos, 1.0);
}