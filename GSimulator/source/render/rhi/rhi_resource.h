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

} // namespace GComponent

#endif // GSIM_RENDER_RHI_RESOURCE_H
