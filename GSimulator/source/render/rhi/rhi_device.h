#ifndef GSIM_RENDER_RHI_DEVICE_H
#define GSIM_RENDER_RHI_DEVICE_H

#include "render/rhi/rhi_resource.h"

#include <cstdint>

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
	virtual void BindTextureUnit(uint32_t unit, RhiTextureHandle texture) = 0;
	virtual void BindDefaultFramebuffer() = 0;
	virtual uint32_t GetDefaultFramebuffer() const = 0;

	virtual RhiBufferHandle CreateBuffer(const RhiBufferDesc& desc, const void* initial_data = nullptr) = 0;
	virtual void UpdateBuffer(RhiBufferHandle buffer, size_t offset, size_t size, const void* data) = 0;
	virtual void DestroyBuffer(RhiBufferHandle buffer) = 0;

	virtual RhiTextureHandle CreateTexture(const RhiTextureDesc& desc, const void* initial_data = nullptr) = 0;
	virtual void DestroyTexture(RhiTextureHandle texture) = 0;
};

} // namespace GComponent

#endif // GSIM_RENDER_RHI_DEVICE_H
