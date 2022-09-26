#version 450 core

struct DirLight{
  vec3 dir;
  vec3 color;
};

// Passing from vertex shader
in vec3 FragPos;
in vec3 Norm;
in vec2 TexCoords;

// Some Predefinition Parameters
#define Ambient     0.1
#define Specular    0.70
#define Diffuse     0.55

// the global parameter use from camera and screen size
layout(std140, set = 0, binding = 0) uniform Matrices{
    mat4 projection;
    mat4 view;
};

// ambient observer include the global direct light and the camera position
layout(std140, set = 0, binding = 1) uniform ambient_observer_parameter{
    DirLight light;
    vec3     viewpos;
};

// the cascade lighting projection and ortho matrices
layout(std140, set = 0, binding = 2) uniform light_space_parameter{
    mat4 light_space_matrices[16];
};

// output color in this pixel / fragment
out vec4 FragColor;

// cascaded shadow map parameters
uniform float           far_plane;
uniform float           cascade_plane[16];
uniform int             csm_levels;
uniform sampler2DArray  shadow_map;

// normally shading parameters
uniform vec3            color;

vec3 CalcLight(DirLight light, vec3 norm, vec3 view_dir);
int  GetShadowLayer(vec3 frag_pos_world_space);
float ShadowCaculation(vec4 frag_pos_light_space, float bias, int layer);

void main(void)
{
    vec3 result   = vec3(0.0f);
    vec3 norm     = normalize(Norm);
    vec3 viewDir  = normalize(viewpos - FragPos);

    result       += CalcLight(light, norm, viewDir);

    FragColor     = vec4(result, 1.0f);
}

vec3 CalcLight(DirLight light, vec3 norm, vec3 view_dir)
{
    vec3 half_way_dir = normalize(light.dir + view_dir);

    // lightting part
    vec3 ambient    = vec3(Ambient);
    vec3 diffuse    = vec3(Diffuse)  * max( dot( light.dir, norm), 0.0f);
    vec3 specular   = vec3(Specular) * pow( max( dot( norm, half_way_dir), 0.0f), 55.0f);
    
    // shadow part
    float bias      = max(0.0015 * (1.0 - dot(norm, light.dir)), 0.001f);
    int   layer     = GetShadowLayer(FragPos);
    vec4  frag_pos_light_space 
                    = light_space_matrices[layer] * vec4(FragPos, 1.0);
    float shadow    = ShadowCaculation(frag_pos_light_space, bias, layer);

    return (ambient + (1.0 - shadow) * (diffuse + specular)) * light.color * color;
}

int GetShadowLayer(vec3 frag_pos_world_space)
{
    vec4    frag_pos_view_space = view * vec4(frag_pos_world_space, 1.0f);
    float   depth_value = abs(frag_pos_view_space.z);

    // get the exact layer
    int layer = csm_levels;
    for(int i = 0; i < csm_levels; ++i)
    {
        if(depth_value < cascade_plane[i]){
            layer = i;
            break;
        }
    }
    return layer;
}

float ShadowCaculation(vec4 frag_pos_light_space, float bias, int layer)
{       
    // as normally shadow map caculations
    vec3 proj_coords          = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords               = proj_coords * 0.5f + 0.5f;    
    float current_depth       = proj_coords.z;

    if(current_depth > 1.0) return 0.0;
    // modifier the bias acoording the layer
    const float bias_modifier = 0.5f;
    if(layer == csm_levels){
        bias *= 1 / ( far_plane * bias_modifier);
    }
    else{
        bias *= 1 / (cascade_plane[layer] * bias_modifier);
    }

    // percentage closest filter
    float shadow     = 0.0f;
    vec2  texel_size = 1.0 / vec2(textureSize(shadow_map, 0));
    for(int x = -2; x <= 2; ++x)for(int y = - 2; y <= 2; ++y)
    {
        float pcf_depth = texture(shadow_map, vec3(proj_coords.xy + vec2(x, y) * texel_size, layer)).r;
        shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
    }

    return shadow / 25.0f;
}
