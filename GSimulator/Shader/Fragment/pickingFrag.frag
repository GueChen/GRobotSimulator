#version 420 core

uniform uint gDrawIndex;
uniform uint gModelIndex;

out vec4 frag_color;

void main(){
	frag_color = vec4(float(gDrawIndex), float(gModelIndex), float(gl_PrimitiveID + 1), 1.0f);
}