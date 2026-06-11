#ifndef __SHADER_PROPERTY_HPP
#define __SHADER_PROPERTY_HPP

#include "render/rendering_datastructure.hpp"
#include "render/rhi/rhi_resource.h"

#include <glm/glm.hpp>

#include <cassert>
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

    [[nodiscard]] RhiMaterialParameterDesc ToParameterDesc() const
    {
        RhiMaterialParameterDesc desc;
        desc.type = ToRhiMaterialParameterType(type);
        desc.type_name = type;
        desc.name = name;
        desc.binding = location;
        return desc;
    }

    static ShaderProperty FromParameterDesc(const RhiMaterialParameterDesc& desc)
    {
        ShaderProperty property;
        property.type = desc.type_name.empty() ? ToString(desc.type) : desc.type_name;
        property.name = desc.name;
        property.location = desc.binding;
        property.val = DefaultValue(desc.type);
        return property;
    }

private:
    static Var DefaultValue(RhiMaterialParameterType type)
    {
        switch (type) {
        case RhiMaterialParameterType::Bool:           return false;
        case RhiMaterialParameterType::Int:            return 0;
        case RhiMaterialParameterType::UnsignedInt:    return 0u;
        case RhiMaterialParameterType::Float:          return 0.0f;
        case RhiMaterialParameterType::Double:         return 0.0;
        case RhiMaterialParameterType::Vec2:           return glm::vec2(0.0f);
        case RhiMaterialParameterType::Vec3:           return glm::vec3(1.0f);
        case RhiMaterialParameterType::Color:          return glm::vec3(Color::White);
        case RhiMaterialParameterType::Vec4:           return glm::vec4(0.0f);
        case RhiMaterialParameterType::Mat2:           return glm::mat2(1.0f);
        case RhiMaterialParameterType::Mat3:           return glm::mat3(1.0f);
        case RhiMaterialParameterType::Mat4:           return glm::mat4(1.0f);
        case RhiMaterialParameterType::Texture2D:
        case RhiMaterialParameterType::TextureCube:
        case RhiMaterialParameterType::Texture2DArray: return Texture{};
        case RhiMaterialParameterType::Unknown:
        default:
            assert(false && "unsupported material parameter type");
            return 0;
        }
    }

};
using ShaderProperties = std::vector<ShaderProperty>;
}
#endif // !__SHADER_PROPERTY_HPP
