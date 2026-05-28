#ifndef GSIM_RENDER_RHI_TYPES_H
#define GSIM_RENDER_RHI_TYPES_H

#include <cstdint>

namespace GComponent {

enum class RhiBackendType {
	OpenGL,
	DirectX12
};

enum class RhiCapability {
	DepthTest,
	Blend,
	Multisample
};

enum class RhiDepthFunc {
	Less,
	LessEqual
};

enum class RhiCullFace {
	Front,
	Back
};

enum class RhiPolygonMode {
	Fill,
	Line
};

enum class RhiPrimitiveTopology {
	Points,
	Lines,
	Triangles
};

enum class RhiClearFlags : uint32_t {
	None = 0,
	Color = 1u << 0,
	Depth = 1u << 1
};

inline RhiClearFlags operator|(RhiClearFlags lhs, RhiClearFlags rhs)
{
	return static_cast<RhiClearFlags>(
		static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

inline bool HasFlag(RhiClearFlags value, RhiClearFlags flag)
{
	return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

struct RhiViewport {
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
};

struct RhiClearColor {
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;
	float a = 1.0f;
};

template <class Tag>
struct RhiHandle {
	uint64_t value = 0;

	[[nodiscard]] bool IsValid() const { return value != 0; }
	explicit operator bool() const { return IsValid(); }
};

struct RhiBufferTag {};
struct RhiTextureTag {};
struct RhiShaderTag {};
struct RhiFramebufferTag {};
struct RhiMeshTag {};

using RhiBufferHandle = RhiHandle<RhiBufferTag>;
using RhiTextureHandle = RhiHandle<RhiTextureTag>;
using RhiShaderHandle = RhiHandle<RhiShaderTag>;
using RhiFramebufferHandle = RhiHandle<RhiFramebufferTag>;
using RhiMeshHandle = RhiHandle<RhiMeshTag>;

} // namespace GComponent

#endif // GSIM_RENDER_RHI_TYPES_H
