#version 420 core

struct DirLight{
  vec3 dir;
  vec3 color;
};

in vec3 FragPos;
in vec3 Norm;
in vec2 TexCoords;

layout(std140, set = 0, binding = 1) uniform ambient_observer_parameter{
    DirLight light;
    vec3     viewpos;
};

out vec4 FragColor;

#define Ambient 0.1
#define Specular 0.70
#define Diffuse 0.55

uniform vec3 color;

vec3 CalcLight(DirLight light, vec3 norm, vec3 viewDir);

void main(void)
{
    vec3 result   = vec3(0.0f);
    vec3 norm     = normalize(Norm);
    vec3 viewDir  = normalize(viewpos - FragPos);

    result       += CalcLight(light, norm, viewDir);

    FragColor     = vec4(result, 1.0f);
    //FragColor = vec4(vec3(gl_FragCoord.z), 1.0f);
}

vec3 CalcLight(DirLight light, vec3 norm, vec3 viewDir)
{
    vec3 halfWayDir = normalize(light.dir + viewDir);

    vec3 ambient    = vec3(Ambient);
    vec3 diffuse    = vec3(Diffuse)  * max( dot( light.dir, norm), 0.0f);
    vec3 specular   = vec3(Specular) * pow( max( dot( norm, halfWayDir), 0.0f), 55.0f);

    return (ambient + diffuse + specular) * light.color * color;
}
