#version 450 core

out vec4 frag_color;

in  vec3 tex_coords;

layout(binding = 3) uniform samplerCube environment_map;

uniform float	   roughness;

const float PI = 3.1415926535;

float RadicalInverse_Vdc(uint bits);
vec2  Hammersley(uint i, uint N);
vec3  ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness);

void main(){
	vec3 N = normalize(tex_coords);
	vec3 R = N;
	vec3 V = R;

	const uint kNrSample    = 1024u;
	float total_weight      = 0.0;
	vec3  prefiltered_color = vec3(0.0);
	for (uint i = 0u; i < kNrSample; ++i){
		vec2 Xi = Hammersley(i, kNrSample);
		vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
		vec3 L  = normalize(2.0 * dot(V, H) * H - V);
		
		float NdotL = max(dot(N, L), 0.0);
		if(NdotL > 0.0){
			prefiltered_color += texture(environment_map, L).rgb * NdotL;
			total_weight      += NdotL;
		}
	}
	
	prefiltered_color = prefiltered_color / total_weight;
	
	frag_color = vec4(prefiltered_color, 1.0);
}

float RadicalInverse_Vdc(uint bits){
	bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_Vdc(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness * roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}  