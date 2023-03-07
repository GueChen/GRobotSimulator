#ifndef __SHADER_PROPERTY_HPP
#define __SHADER_PROPERTY_HPP

#include <glm/glm.hpp>

#include <variant>
#include <vector>
#include <string>

struct Property {
    // Types alias
    using sampler2D = unsigned int;
    using samplerCUBE = unsigned int;
    using sampler2DArray = unsigned int;
    using Var = std::variant<
        bool,
        int,
        unsigned int,
        float,
        double,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        glm::mat2,
        glm::mat3,
        glm::mat4>;

    // Fieds
    std::string type;
    std::string name;
    int         location;    
    Var         val;

};
using Properties = std::vector<Property>;

#endif // !__SHADER_PROPERTY_HPP

