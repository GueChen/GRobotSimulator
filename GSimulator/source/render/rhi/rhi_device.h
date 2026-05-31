#ifndef GSIM_RENDER_RHI_DEVICE_H
#define GSIM_RENDER_RHI_DEVICE_H

#include "render/rhi/rhi_resource.h"

#include <cstdint>
#include <string_view>
#include <vector>

namespace GComponent {

class IRhiDevice {
public:
	virtual ~IRhiDevice() = default;

	[[nodiscard]] virtual RhiBackendType GetBackendType() const = 0;
	virtual void Initialize() = 0;

	virtual void Enable(RhiCapability capability) = 0;
	virtual void Disable(RhiCapability capability) = 0;
	virtual void SetDepthFunc(RhiDepthFunc func) = 0;
	virtual void SetCullFace(RhiCullFace face) = 0;
	virtual void SetPolygonMode(RhiPolygonMode mode) = 0;
	virtual void SetBlendAlpha() = 0;
	virtual void SetLineWidth(float width) = 0;
	virtual void SetViewport(const RhiViewport& viewport) = 0;
	virtual void SetClearColor(const RhiClearColor& color) = 0;
	virtual void Clear(RhiClearFlags flags) = 0;
	virtual void PushDebugGroup(std::string_view name) = 0;
	virtual void PopDebugGroup() = 0;
	virtual void BindTextureUnit(uint32_t unit, RhiTextureHandle texture) = 0;
	virtual void BindDefaultFramebuffer() = 0;
	virtual void BindDefaultFramebuffer(RhiFramebufferBindTarget target) = 0;
	virtual void BindFramebuffer(RhiFramebufferBindTarget target, RhiFramebufferHandle framebuffer) = 0;
	virtual uint32_t GetDefaultFramebuffer() const = 0;

	virtual RhiBufferHandle CreateBuffer(const RhiBufferDesc& desc, const void* initial_data = nullptr) = 0;
	virtual void BindBuffer(RhiBufferUsage usage, RhiBufferHandle buffer) = 0;
	virtual void BindUniformBufferBase(uint32_t binding, RhiBufferHandle buffer) = 0;
	virtual void UpdateBuffer(RhiBufferHandle buffer, size_t offset, size_t size, const void* data) = 0;
	virtual void DestroyBuffer(RhiBufferHandle buffer) = 0;

	virtual RhiTextureHandle CreateTexture(const RhiTextureDesc& desc, const void* initial_data = nullptr) = 0;
	virtual void DestroyTexture(RhiTextureHandle texture) = 0;
	virtual RhiTextureHandle LoadTexture2D(std::string_view path, bool repeat = true) = 0;
	virtual RhiTextureHandle LoadCubemap(const std::vector<std::string_view>& paths) = 0;

	virtual RhiFramebufferHandle CreatePickingFramebuffer(int width, int height) = 0;
	virtual RhiFramebufferHandle CreateFramebuffer(const RhiFramebufferCreateDesc& desc) = 0;
	virtual void DestroyFramebuffer(RhiFramebufferHandle framebuffer) = 0;
	virtual RhiTextureHandle GetFramebufferTexture(RhiFramebufferHandle framebuffer) const = 0;
	virtual RhiTextureHandle GetFramebufferColorTexture(RhiFramebufferHandle framebuffer, uint32_t index) const = 0;
	virtual RhiTextureHandle TakeFramebufferTexture(RhiFramebufferHandle framebuffer) = 0;
	virtual RhiTextureHandle ReallocateFramebufferTexture(RhiFramebufferHandle framebuffer, const RhiFramebufferCreateDesc& desc) = 0;
	virtual void ResizeFramebufferRenderbuffer(RhiFramebufferHandle framebuffer, int width, int height) = 0;
	virtual void ReadFramebufferColorPixel(RhiFramebufferHandle framebuffer, uint32_t x, uint32_t y, float* rgb) = 0;

	virtual RhiMeshHandle CreateMesh(const RhiMeshDesc& desc) = 0;
	virtual void UpdateMeshVertexData(RhiMeshHandle mesh, size_t offset, size_t size, const void* data) = 0;
	virtual void DestroyMesh(RhiMeshHandle mesh) = 0;
	virtual void DrawMesh(RhiMeshHandle mesh, RhiPrimitiveTopology topology, uint32_t index_count) = 0;
};

} // namespace GComponent

#endif // GSIM_RENDER_RHI_DEVICE_H
