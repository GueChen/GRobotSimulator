#version 420 core

float near = 0.1f;
float far  = 100.0f;

float Linearize(float depth)
{
	float z = depth * 2.0f - 1.0f;
	return (2.0f * near * far) / (far + near - z * (far - near));
}

void main()
{
	//gl_FragDepth = Linearize(gl_FragCoord.z) / far;
}