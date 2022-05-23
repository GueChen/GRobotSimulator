/**
 *  @file  	rendering_datastructure.hpp
 *  @brief 	The Basic .
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Oct   25, 2021
 *  @update April 11, 2022 modifier the type to Triangle to adpater interface
 *          May   12, 2022 add hash optimize reading reference from vulkan tutorial
 **/
#ifndef _RENDERING_DATASTRUCTURE_HPP
#define _RENDERING_DATASTRUCTURE_HPP
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <type_traits>
#include <string>

namespace GComponent{

using vec4 = glm::vec4;
using vec3 = glm::vec3;
using vec2 = glm::vec2;

struct Line{
    int first;
    int second;
};

struct Triangle {
    int first;
    int second;
    int third;
};

struct Vertex {
    vec3 Position;
    vec3 Normal;
    vec2 TexCoords;

    bool operator==(const Vertex& other) const {
        return Position == other.Position && Normal == other.Normal && TexCoords == other.TexCoords;
    }
};

struct ColorVertex :public Vertex {
    vec3 Color;
};

struct Texture {
    unsigned id;
    std::string type;
};

}

namespace std {
template<>
struct hash<GComponent::Vertex> {
    size_t operator()(GComponent::Vertex const& vertex) const
    {
        return ((hash<glm::vec3>()(vertex.Position) ^
                (hash<glm::vec3>()(vertex.Normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.TexCoords) << 1);
    }
};

}

#endif // rendering_datastructure_HPP
