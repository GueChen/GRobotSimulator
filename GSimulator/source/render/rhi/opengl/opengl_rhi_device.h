#ifndef GSIM_RENDER_RHI_OPENGL_DEVICE_H
#define GSIM_RENDER_RHI_OPENGL_DEVICE_H

#include "render/rhi/rhi_device.h"
#include "render/mygl.hpp"

#include <memory>
#include <unordered_map>

namespace GComponent {

class OpenGLRhiDevice final : public IRhiDevice {
public:
	OpenGLRhiDevice();
	explicit OpenGLRhiDevice(std::shared_ptr<MyGL> gl);

	[[nodiscard]] RhiBackendType GetBackendType() const override;
	void Initialize() override;

	void Enable(RhiCapability capability) override;
	void Disable(RhiCapability capability) override;
	void SetDepthFunc(RhiDepthFunc func) override;
	void SetCullFace(RhiCullFace face) override;
	void SetPolygonMode(RhiPolygonMode mode) override;
	void SetBlendAlpha() override;
	void SetLineWidth(float width) override;
	void SetViewport(const RhiViewport& viewport) override;
	void SetClearColor(const RhiClearColor& color) override;
	void Clear(RhiClearFlags flags) override;
	void BindTextureUnit(uint32_t unit, RhiTextureHandle texture) override;
	void BindDefaultFramebuffer() override;
	uint32_t GetDefaultFramebuffer() const override;

	RhiBufferHandle CreateBuffer(const RhiBufferDesc& desc, const void* initial_data = nullptr) override;
	void UpdateBuffer(RhiBufferHandle buffer, size_t offset, size_t size, const void* data) override;
	void DestroyBuffer(RhiBufferHandle buffer) override;

	RhiTextureHandle CreateTexture(const RhiTextureDesc& desc, const void* initial_data = nullptr) override;
	void DestroyTexture(RhiTextureHandle texture) override;

	[[nodiscard]] const std::shared_ptr<MyGL>& GetGL() const { return gl_; }
	[[nodiscard]] MyGL* GetGLRaw() const { return gl_.get(); }

private:
	static unsigned ToGLCapability(RhiCapability capability);
	static unsigned ToGLDepthFunc(RhiDepthFunc func);
	static unsigned ToGLCullFace(RhiCullFace face);
	static unsigned ToGLPolygonMode(RhiPolygonMode mode);
	static unsigned ToGLClearFlags(RhiClearFlags flags);
	static unsigned ToGLBufferTarget(RhiBufferUsage usage);
	static unsigned ToGLTextureTarget(RhiTextureDimension dimension);
	static unsigned ToGLInternalFormat(RhiTextureFormat format);
	static unsigned ToGLFormat(RhiTextureFormat format);
	static unsigned ToGLType(RhiTextureFormat format);

private:
	std::shared_ptr<MyGL> gl_;
	std::unordered_map<uint64_t, RhiBufferUsage> buffer_usages_;
};

std::shared_ptr<OpenGLRhiDevice> AsOpenGLRhiDevice(const std::shared_ptr<IRhiDevice>& device);

} // namespace GComponent

#endif // GSIM_RENDER_RHI_OPENGL_DEVICE_H
