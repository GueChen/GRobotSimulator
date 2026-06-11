#ifndef GSIM_RENDER_RHI_TYPES_H
#define GSIM_RENDER_RHI_TYPES_H

#include <cstdint>

namespace GComponent {

enum class RhiBackendType {
	OpenGL,
	DirectX12
};

enum class RhiAdapterPreference {
	Default,
	HighPerformance,
	LowPower
};

enum class RhiDeviceFeature {
	DebugGroups,
	Texture2DArray,
	TextureCubemap,
	MultipleColorAttachments,
	FramebufferReadback
};

enum class RhiCapability {
	DepthTest,
	Blend,
	Multisample,
	CullFace
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

enum class RhiFramebufferBindTarget {
	Framebuffer,
	Draw,
	Read
};

enum class RhiPrimitiveTopology {
	Points,
	Lines,
	LineStrip,
	Triangles
};

enum class RhiNativeSurfaceType {
	None,
	Win32Hwnd
};

enum class RhiDefaultRenderTargetOwnership {
	External,
	Backend
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

struct RhiNativeSurface {
	RhiNativeSurfaceType type = RhiNativeSurfaceType::None;
	void* handle = nullptr;

	[[nodiscard]] bool IsValid() const { return handle != nullptr; }
};

struct RhiPresentSurfaceDesc {
	RhiNativeSurface native_surface{};
	uint32_t width = 0;
	uint32_t height = 0;
	RhiDefaultRenderTargetOwnership default_render_target_ownership = RhiDefaultRenderTargetOwnership::External;

	[[nodiscard]] bool HasNativeSurface() const { return native_surface.IsValid(); }
};

struct RhiDeviceInitConfig {
	RhiBackendType backend = RhiBackendType::OpenGL;
	RhiAdapterPreference adapter_preference = RhiAdapterPreference::HighPerformance;
	bool enable_debug_layer = false;
	bool enable_validation = false;
	RhiPresentSurfaceDesc present_surface{};
};

struct RhiDeviceCapabilities {
	RhiBackendType backend = RhiBackendType::OpenGL;
	bool supports_debug_groups = false;
	bool supports_texture_2d_array = false;
	bool supports_texture_cubemap = false;
	bool supports_multiple_color_attachments = false;
	bool supports_framebuffer_readback = false;
	uint32_t max_color_attachments = 1;
	uint32_t max_texture_array_layers = 1;
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
