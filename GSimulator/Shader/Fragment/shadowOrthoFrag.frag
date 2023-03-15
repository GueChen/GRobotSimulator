#version 420 core

struct DirLight{
  vec3 dir;
  vec3 color;
};

in vec3 FragPos;
in vec3 Norm;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

layout(std140, set = 0, binding = 1) uniform ambient_observer_parameter{
    DirLight light;
    vec3     viewpos;
};

layout(binding = 3) uniform sampler2DArray direct_light_shadow_map;

out vec4 FragColor;

#define Ambient 0.1
#define Specular 0.70
#define Diffuse 0.55

//uniform sampler2D   shadow_map;
uniform vec3        color;

vec3    CalcLight(DirLight light, vec3 norm, vec3 viewDir);
float   ShadowCalculation(vec4 frag_pos_light_space, float bias);

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
    // lightting part
    vec3 ambient    = vec3(Ambient);
    vec3 diffuse    = vec3(Diffuse)  * max( dot( light.dir, norm), 0.0f);
    vec3 specular   = vec3(Specular) * pow( max( dot( norm, halfWayDir), 0.0f), 55.0f);
    
    // shadow part
    float bias      = max(0.005 * (1.0 - dot(norm, light.dir)), 0.0005f);
    float shadow    = ShadowCalculation(FragPosLightSpace, bias);

    return (ambient + (1.0 - shadow) * (diffuse + specular)) * light.color * color;
}

float ShadowCalculation(vec4 frag_pos_light_space, float bias)
{
    vec3 proj_coords    = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords         = proj_coords * 0.5f + 0.5f;    
    float current_depth = proj_coords.z;

    float shadow        = 0;    
    vec2  texel_size     = 1.0 / textureSize(direct_light_shadow_map, 0).xy;
    for(int x = -1; x <= 1; ++x)for(int y = - 1; y <= 1; ++y)
    {
        float pcf_depth = texture(direct_light_shadow_map, vec3(proj_coords.xy + vec2(x, y) * texel_size, 0)).r;
        shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
    }
    
    return shadow / 9.0f;
}
