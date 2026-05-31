#ifndef GSIM_RENDER_RHI_RESOURCE_H
#define GSIM_RENDER_RHI_RESOURCE_H

#include "render/rhi/rhi_types.h"

#include <cstddef>
#include <string>
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
	Rg16Float,
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
	std::string path;
};

struct RhiShaderDesc {
	std::string name;
	RhiShaderStageDesc vertex;
	RhiShaderStageDesc fragment;
	RhiShaderStageDesc geometry;
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

} // namespace GComponent

#endif // GSIM_RENDER_RHI_RESOURCE_H
