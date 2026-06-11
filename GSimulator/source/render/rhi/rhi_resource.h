#ifndef GSIM_RENDER_RHI_RESOURCE_H
#define GSIM_RENDER_RHI_RESOURCE_H

#include "render/rhi/rhi_types.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace GComponent {

enum class RhiBufferUsage {
	Vertex,
	Index,
	Uniform
};

enum class RhiTextureDimension {
	Texture2D,
	Texture2DArray,
	TextureCube
};

enum class RhiTextureFormat {
	Rgb32Float,
	Rgb16Float,
	Rgba16Float,
	Rg16Float,
	Rgba8Unorm,
	Depth32Float,
	Depth24Stencil8
};

enum class RhiVertexLayout {
	Position3,
	PositionNormalTexcoord,
	PositionNormalTexcoordColor
};

enum class RhiFramebufferAttachment {
	Color,
	Depth,
	Cube,
	CubeMipmap
};

struct RhiBufferDesc {
	RhiBufferUsage usage = RhiBufferUsage::Vertex;
	size_t size = 0;
};

struct RhiTextureDesc {
	RhiTextureDimension dimension = RhiTextureDimension::Texture2D;
	RhiTextureFormat format = RhiTextureFormat::Rgb32Float;
	int width = 0;
	int height = 0;
	int layers = 1;
	int mip_levels = 1;
};

struct RhiShaderStageDesc {
	RhiShaderStageDesc() = default;
	RhiShaderStageDesc(std::string shader_path)
		: path(std::move(shader_path))
	{
	}

	std::string path;

	[[nodiscard]] bool IsValid() const
	{
		return !path.empty();
	}
};

enum class RhiMaterialParameterType {
	Unknown,
	Bool,
	Int,
	UnsignedInt,
	Float,
	Double,
	Vec2,
	Vec3,
	Color,
	Vec4,
	Mat2,
	Mat3,
	Mat4,
	Texture2D,
	TextureCube,
	Texture2DArray
};

struct RhiShaderDesc {
	std::string name;
	RhiBackendType backend = RhiBackendType::OpenGL;
	RhiShaderStageDesc vertex;
	RhiShaderStageDesc fragment;
	RhiShaderStageDesc geometry;

	[[nodiscard]] bool HasGeometryStage() const
	{
		return geometry.IsValid();
	}
};

struct RhiMaterialParameterDesc {
	RhiMaterialParameterType type = RhiMaterialParameterType::Unknown;
	std::string type_name;
	std::string name;
	int binding = -1;
};

using RhiMaterialParameters = std::vector<RhiMaterialParameterDesc>;

struct RhiMaterialDesc {
	std::string shader_name;
	RhiBackendType backend = RhiBackendType::OpenGL;
	RhiMaterialParameters parameters;
};

struct RhiFramebufferDesc {
	RhiTextureHandle color;
	RhiTextureHandle depth;
	int width = 0;
	int height = 0;
};

struct RhiFramebufferCreateDesc {
	int width = 0;
	int height = 0;
	int layers = 0;
	RhiFramebufferAttachment attachment = RhiFramebufferAttachment::Color;
	std::vector<RhiTextureFormat> color_attachments;
};

struct RhiMeshDesc {
	const void* vertex_data = nullptr;
	size_t vertex_data_size = 0;
	size_t vertex_count = 0;
	size_t vertex_stride = 0;
	const void* index_data = nullptr;
	size_t index_data_size = 0;
	size_t index_count = 0;
	RhiVertexLayout vertex_layout = RhiVertexLayout::Position3;
};

inline const char* ToString(RhiMaterialParameterType type)
{
	switch (type) {
	case RhiMaterialParameterType::Bool:           return "bool";
	case RhiMaterialParameterType::Int:            return "int";
	case RhiMaterialParameterType::UnsignedInt:    return "unsigned int";
	case RhiMaterialParameterType::Float:          return "float";
	case RhiMaterialParameterType::Double:         return "double";
	case RhiMaterialParameterType::Vec2:           return "vec2";
	case RhiMaterialParameterType::Vec3:           return "vec3";
	case RhiMaterialParameterType::Color:          return "color";
	case RhiMaterialParameterType::Vec4:           return "vec4";
	case RhiMaterialParameterType::Mat2:           return "mat2";
	case RhiMaterialParameterType::Mat3:           return "mat3";
	case RhiMaterialParameterType::Mat4:           return "mat4";
	case RhiMaterialParameterType::Texture2D:      return "sampler2D";
	case RhiMaterialParameterType::TextureCube:    return "samplerCUBE";
	case RhiMaterialParameterType::Texture2DArray: return "sampler2DArray";
	case RhiMaterialParameterType::Unknown:
	default:
		return "?";
	}
}

inline RhiMaterialParameterType ToRhiMaterialParameterType(std::string_view type_name)
{
	if (type_name == "bool") return RhiMaterialParameterType::Bool;
	if (type_name == "int") return RhiMaterialParameterType::Int;
	if (type_name == "unsigned int") return RhiMaterialParameterType::UnsignedInt;
	if (type_name == "float") return RhiMaterialParameterType::Float;
	if (type_name == "double") return RhiMaterialParameterType::Double;
	if (type_name == "vec2") return RhiMaterialParameterType::Vec2;
	if (type_name == "vec3") return RhiMaterialParameterType::Vec3;
	if (type_name == "color") return RhiMaterialParameterType::Color;
	if (type_name == "vec4") return RhiMaterialParameterType::Vec4;
	if (type_name == "mat2") return RhiMaterialParameterType::Mat2;
	if (type_name == "mat3") return RhiMaterialParameterType::Mat3;
	if (type_name == "mat4") return RhiMaterialParameterType::Mat4;
	if (type_name == "sampler2D") return RhiMaterialParameterType::Texture2D;
	if (type_name == "samplerCUBE") return RhiMaterialParameterType::TextureCube;
	if (type_name == "sampler2DArray") return RhiMaterialParameterType::Texture2DArray;
	return RhiMaterialParameterType::Unknown;
}

inline bool IsTextureMaterialParameterType(RhiMaterialParameterType type)
{
	return type == RhiMaterialParameterType::Texture2D ||
		   type == RhiMaterialParameterType::TextureCube ||
		   type == RhiMaterialParameterType::Texture2DArray;
}

inline RhiShaderDesc MakeOpenGlShaderDesc(std::string name,
										  std::string vertex_path,
										  std::string fragment_path,
										  std::string geometry_path = {})
{
	RhiShaderDesc desc;
	desc.name = std::move(name);
	desc.backend = RhiBackendType::OpenGL;
	desc.vertex = RhiShaderStageDesc{ std::move(vertex_path) };
	desc.fragment = RhiShaderStageDesc{ std::move(fragment_path) };
	desc.geometry = RhiShaderStageDesc{ std::move(geometry_path) };
	return desc;
}

} // namespace GComponent

#endif // GSIM_RENDER_RHI_RESOURCE_H
