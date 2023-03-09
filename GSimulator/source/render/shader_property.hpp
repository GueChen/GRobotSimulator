#ifndef __SHADER_PROPERTY_HPP
#define __SHADER_PROPERTY_HPP

#include "render/rendering_datastructure.hpp"

#include <glm/glm.hpp>

#include <variant>
#include <vector>
#include <string>

namespace GComponent{
struct ShaderProperty {
    // Types alias
    using sampler2D      = unsigned int;
    using samplerCUBE    = unsigned int;
    using sampler2DArray = unsigned int;
    using Var = std::variant<
        bool,
        int,
        unsigned int,
        float,
        double,
        glm::vec2,
        glm::vec3,
        Color,
        glm::vec4,
        glm::mat2,
        glm::mat3,
        glm::mat4,
        Texture>;

    // Fieds
    std::string type;
    std::string name;
    int         location;
    Var         val;

};
using ShaderProperties = std::vector<ShaderProperty>;
}
#endif // !__SHADER_PROPERTY_HPP

