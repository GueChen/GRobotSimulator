struct DirLight {
    vec3 dir;
    vec3 color;
};

layout(std140, set = 0, binding = 0) uniform Matrices {
    mat4 projection;
    mat4 view;
};

// ambient observer include the global direct light and the camera position
layout(std140, set = 0, binding = 1) uniform ambient_observer_parameter {
    DirLight light;
    vec3     viewpos;
    float    near_plane;
    float    far_plane;
};

// the cascade lighting projection and ortho matrices
layout(std140, set = 0, binding = 2) uniform light_space_parameter {
    mat4         light_space_matrices[16];
    float        cascade_plane[16];
    int          csm_levels;
};

//layout(bindless_sampler) uniform;
layout(binding = 3) uniform sampler2DArray direct_light_shadow_map;
