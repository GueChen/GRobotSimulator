#version 420 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aTexCoords;

layout (std140, binding = 0) uniform Matrices{
	mat4 projection;
	mat4 view;
};

uniform mat4 model;
uniform int selected;

out vec3 in_color;

void main(){
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	
	if(aTexCoords.x < 0.01f){
		if(selected == 0){
			in_color = vec3(1.0, 1.0, 1.0);
		}
		else{
			in_color = vec3(1.0, 0.0, 0.0);
		}
	}else if(aTexCoords.x < 1.01f){
		if(selected == 1){
			in_color = vec3(1.0, 1.0, 1.0);
		}
		else{
			in_color = vec3(0.0, 1.0, 0.0);
		}
	}else if(aTexCoords.x < 2.01f){
		if(selected == 2){
			in_color = vec3(1.0, 1.0, 1.0);
		}
		else{
			in_color = vec3(0.0, 0.0, 1.0);
		}
	}
	
}