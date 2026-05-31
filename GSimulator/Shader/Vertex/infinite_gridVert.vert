#version 450 core

layout(location = 0) in vec3 aPos;

layout(std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out vec3 farPoint;
flat out vec3 cameraPosition;

vec3 UnprojectPoint(float x, float y, float z)
{
    mat4 invViewProj = inverse(projection * view);
    vec4 unprojected = invViewProj * vec4(x, y, z, 1.0);
    return unprojected.xyz / unprojected.w;
}

void main()
{
    farPoint = UnprojectPoint(aPos.x, aPos.y, 1.0);
    cameraPosition = inverse(view)[3].xyz;
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}
