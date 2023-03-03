#version 420 core

in vec3 in_color;

out vec4 frag_color;

void main(){
	frag_color = vec4(in_color, 1.0);
}