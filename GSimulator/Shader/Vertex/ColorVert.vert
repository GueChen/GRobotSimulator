#version 420 core

struct DirLight{
  vec3 dir;
  vec3 color;
};

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNorm;
layout(location = 2) in vec2 aTexCoords;

layout(std140, set = 0, binding = 0) uniform Matrices{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;
uniform bool NormReverse;

out vec3 FragPos;
out vec3 Norm;
out vec2 TexCoords;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0f));
    Norm = transpose(inverse(mat3(model))) * aNorm;
    if(NormReverse)
    {
        Norm = - Norm;
    }
    TexCoords = aTexCoords;
    gl_Position = projection *                      // P matrix
                  view *                            // V matrix
                  vec4(FragPos, 1.0f);              // M matrix
}
