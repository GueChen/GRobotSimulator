#ifndef VERTEX_HPP
#define VERTEX_HPP
#include <glm/glm.hpp>

namespace GComponent {
/// TODO： 替换库时请替换这里
    using vec3 = glm::vec3;
    using vec2 = glm::vec2;

    struct Vertex{
        vec3 Position;
        vec3 Normal;
        vec2 TexCoords;
    };
}

#endif // VERTEX_HPP
