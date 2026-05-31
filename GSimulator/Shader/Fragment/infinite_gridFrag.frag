#version 450 core

layout(std140, binding = 0) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

in vec3 farPoint;
flat in vec3 cameraPosition;

out vec4 frag_color;

uniform float gridSize;
uniform float majorGridSize;
uniform float fadeDistance;
uniform float depthBias;
uniform vec3 minorLineColor;
uniform vec3 majorLineColor;
uniform vec3 xAxisColor;
uniform vec3 yAxisColor;

float GridLine(vec2 coord, float scale)
{
    vec2 derivative = max(fwidth(coord / scale), vec2(0.000001));
    vec2 grid = abs(fract(coord / scale - 0.5) - 0.5) / derivative;
    return 1.0 - min(min(grid.x, grid.y), 1.0);
}

float AxisLine(float coord, float scale)
{
    float derivative = max(fwidth(coord / scale), 0.000001);
    return 1.0 - min(abs(coord / scale) / derivative, 1.0);
}

float Depth(vec3 pos)
{
    vec4 clip = projection * view * vec4(pos, 1.0);
    float ndcDepth = clip.z / clip.w;
    return ndcDepth * 0.5 + 0.5;
}

void main()
{
    vec3 rayDirection = normalize(farPoint - cameraPosition);
    if (abs(rayDirection.z) < 0.00001) {
        discard;
    }

    float t = -cameraPosition.z / rayDirection.z;
    if (t <= 0.0) {
        discard;
    }

    vec3 fragPos = cameraPosition + t * rayDirection;
    vec2 coord = fragPos.xy;

    float minor = GridLine(coord, gridSize);
    float major = GridLine(coord, majorGridSize);
    float xAxis = AxisLine(coord.y, gridSize);
    float yAxis = AxisLine(coord.x, gridSize);
    float line = max(max(minor * 0.45, major), max(xAxis, yAxis));
    if (line <= 0.001) {
        discard;
    }

    vec3 color = mix(minorLineColor, majorLineColor, major);
    color = mix(color, xAxisColor, xAxis);
    color = mix(color, yAxisColor, yAxis);

    float distanceFade = 1.0 - smoothstep(fadeDistance * 0.35, fadeDistance, length(fragPos - cameraPosition));
    float alpha = line * distanceFade * 0.55;
    if (alpha <= 0.001) {
        discard;
    }

    gl_FragDepth = min(Depth(fragPos) + depthBias, 1.0);
    frag_color = vec4(color, alpha);
}
