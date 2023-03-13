#version 450 core
#extension GL_KHR_vulkan_glsl: enable

out vec4 frag_color;

in  vec2 tex_coords;
in  vec3 world_pos;
in  vec3 normal;

struct DirLight{
  vec3 dir;
  vec3 color;
};
/*_________________________UNIFORM VARIABLES_____________________________________*/
// the global parameter use from camera and screen size
layout(std140, set = 0, binding = 0) uniform Matrices{
    mat4 projection;
    mat4 view;
};

// ambient observer include the global direct light and the camera position
layout(std140, set = 0, binding = 1) uniform ambient_observer_parameter{
    DirLight light;
    vec3     viewpos;
    float    near_plane;
    float    far_plane;
};

// the cascade lighting projection and ortho matrices
layout(std140, set = 0, binding = 2) uniform light_space_parameter{
    mat4         light_space_matrices[16];
    float        cascade_plane[16];
    int          csm_levels;
};

//layout(bindless_sampler) uniform;
layout(binding = 3) uniform sampler2DArray direct_light_shadow_map;

// environment relative
layout(binding = 4) uniform samplerCube irradiance_map;
layout(binding = 5) uniform samplerCube prefilter_map;
layout(binding = 6) uniform sampler2D   brdf_LUT;

// material relative
uniform vec3  albedo_color;
uniform float metallic;
uniform float roughness;
uniform float ao;


/*_________________________CONSTANT VARIABLES_____________________________________*/
const float PI = 3.14159265359;

/*_________________________Function Declarations__________________________________*/
// caculate fresnel term
vec3  FresnelSchlick(float cos_theta, vec3 F0, float roughness);
// caculate normal distibution term
float DistributionGGX(vec3 N, vec3 H, float roughness);
// support function with geometry term
float GeometrySchlickGGX(float NdotV, float k);
// caculate geometry term
float GeomtrySmith(vec3 N, vec3 V, vec3 L, float roughness);
// light diffuse and specular raddiance
vec3  CaculateRadiance(vec3 radiance_in, vec3 L, vec3 H, vec3 N, vec3 V, vec3 F0);
// ambient reflectance term
vec3  CaculateAmbientRadiance(vec3 N, vec3 V, vec3 R, vec3 F0);

void main(){
    vec3 N = normalize(normal);
    vec3 V = normalize(viewpos - world_pos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04); F0 = mix(F0, albedo_color, metallic);
    
    vec3 radiance_out = vec3(0.0);
    // caculate direct light 
    {
        vec3 L = normalize(light.dir);
        vec3 H = normalize(L + V);
        radiance_out += CaculateRadiance(light.color, L, H, N, V, F0);
    }

    // caculate ambient term   
    vec3 ambient = CaculateAmbientRadiance(N, V, R, F0);
    vec3 color   = ambient + radiance_out;
    // HDR transform
    color = pow(color / (color + vec3(1.0)), vec3(1.0/ 2.2));

    frag_color = vec4(color, 1.0);
}

vec3 CaculateRadiance(vec3 radiance_in, vec3 L, vec3 H, vec3 N, vec3 V, vec3 F0){
    vec3  f = FresnelSchlick(max(dot(H, V), 0.0), F0, roughness);
    float n = DistributionGGX(N, H, roughness);
    float g = GeomtrySmith(N, V, L, roughness);

    vec3  nom      = n * f * g;
    float denom    = 4 * (max(dot(V, N), 0.0)) * max(dot(L, N), 0.0) + 0.0001;
    vec3  specular = nom / denom;

    vec3  Ks    = f;
    vec3  Kd    = vec3(1.0) - Ks;
    float NdotL = max(dot(N, L), 0.0);

    return (Kd * albedo_color / PI + specular) * radiance_in * NdotL;
}

vec3 CaculateAmbientRadiance(vec3 N, vec3 V, vec3 R, vec3 F0){
    vec3 f  = FresnelSchlick(max(dot(N, V), 0.0), F0, roughness);
    vec3 Ks = f;
    vec3 Kd = vec3(1.0) - Ks;
    Kd *= 1.0 - metallic;

    vec3 irradiance = texture(irradiance_map, N).rgb;
    vec3 diffuse    = irradiance * albedo_color;
    
    const float kMaxReflectionLod = 4.0f;
    vec3 prefilter_color 
                  = textureLod(prefilter_map, R, roughness * kMaxReflectionLod).rgb;
    vec2 brdf     = texture(brdf_LUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilter_color * (f * brdf.x + brdf.y);

    return (Kd * diffuse + specular) * ao;
}

/*____________________________________ Helper Function Implementations ________________*/
vec3 FresnelSchlick(float cos_theta, vec3 F0, float roughness){
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 * cos_theta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness){
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom    = a2;
    float denom  = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k){
    float nom    = NdotV;
    float denom  = NdotV * (1 - k) + k;

    return nom / denom;
}

float GeomtrySmith(vec3 N, vec3 V, vec3 L, float roughness){
    float a = roughness + 1.0;
    float k = (a * a) / 8.0;

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float ggx1   = GeometrySchlickGGX(NdotV, k);
    float ggx2   = GeometrySchlickGGX(NdotL, k);

    return ggx1 * ggx2;
}