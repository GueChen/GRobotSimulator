#version 450 core

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedoAo;
layout(location = 3) out vec4 gMaterial;

in vec3 world_pos;
in vec3 normal;
in vec2 tex_coords;

uniform vec3 albedo_color;
uniform float metallic;
uniform float roughness;
uniform float ao;
uniform bool accept_shadow;

void main()
{
    gPosition = vec4(world_pos, 1.0);
    gNormal = vec4(normalize(normal), 1.0);
    gAlbedoAo = vec4(albedo_color, ao);
    gMaterial = vec4(metallic, roughness, accept_shadow ? 1.0 : 0.0, 0.0);
}
