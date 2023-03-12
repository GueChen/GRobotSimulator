#version 450 core

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

layout(std140, set = 0, binding = 2) uniform light_space_parameter{
    mat4         light_space_matrices[16];
    float        cascade_plane[16];
    int          csm_levels;
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
