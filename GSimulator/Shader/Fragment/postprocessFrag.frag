#version 420 core

in vec2 TexCoords;

layout(binding = 8) uniform sampler2D screen_texture;
layout(binding = 9) uniform sampler2D outline_mask;

out vec4 FragColor;

void main()
{	
	vec3 scene_color = texture(screen_texture, TexCoords).rgb;
	vec2 texel_size = 1.0 / vec2(textureSize(outline_mask, 0));
	float center_mask = texture(outline_mask, TexCoords).r;
	float nearby_mask = 0.0;
	const int radius = 3;
	for (int y = -radius; y <= radius; ++y) {
		for (int x = -radius; x <= radius; ++x) {
			vec2 offset = vec2(float(x), float(y));
			if (dot(offset, offset) <= float(radius * radius)) {
				vec2 sample_uv = clamp(TexCoords + offset * texel_size, vec2(0.0), vec2(1.0));
				nearby_mask = max(nearby_mask, texture(outline_mask, sample_uv).r);
			}
		}
	}

	float outline = (center_mask < 0.5 && nearby_mask > 0.5) ? 1.0 : 0.0;
	vec3 outline_color = vec3(1.0, 0.72, 0.08);
	FragColor = vec4(mix(scene_color, outline_color, outline), 1.0);
}