#version 450 core

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

layout (std140, binding = 2) uniform light_space_parameters{
	mat4 light_space_matrices[16];
};

void main()
{
//  Every time Process one Triangle
	for(int i = 0; i < 3; ++i)
	{	
		gl_Position = light_space_matrices[gl_InvocationID] * gl_in[i].gl_Position;
		gl_Layer    = gl_InvocationID;
		EmitVertex();
	}
	EndPrimitive();
}
