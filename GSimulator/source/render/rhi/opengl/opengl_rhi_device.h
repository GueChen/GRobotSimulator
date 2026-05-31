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
	void BindDefaultFramebuffer(RhiFramebufferBindTarget target) override;
	void BindFramebuffer(RhiFramebufferBindTarget target, RhiFramebufferHandle framebuffer) override;
	uint32_t GetDefaultFramebuffer() const override;

	RhiBufferHandle CreateBuffer(const RhiBufferDesc& desc, const void* initial_data = nullptr) override;
	void BindBuffer(RhiBufferUsage usage, RhiBufferHandle buffer) override;
	void BindUniformBufferBase(uint32_t binding, RhiBufferHandle buffer) override;
	void UpdateBuffer(RhiBufferHandle buffer, size_t offset, size_t size, const void* data) override;
	void DestroyBuffer(RhiBufferHandle buffer) override;

	RhiTextureHandle CreateTexture(const RhiTextureDesc& desc, const void* initial_data = nullptr) override;
	void DestroyTexture(RhiTextureHandle texture) override;
	RhiTextureHandle LoadTexture2D(std::string_view path, bool repeat = true) override;
	RhiTextureHandle LoadCubemap(const std::vector<std::string_view>& paths) override;

	RhiFramebufferHandle CreatePickingFramebuffer(int width, int height) override;
	RhiFramebufferHandle CreateFramebuffer(const RhiFramebufferCreateDesc& desc) override;
	void DestroyFramebuffer(RhiFramebufferHandle framebuffer) override;
	RhiTextureHandle GetFramebufferTexture(RhiFramebufferHandle framebuffer) const override;
	RhiTextureHandle GetFramebufferColorTexture(RhiFramebufferHandle framebuffer, uint32_t index) const override;
	RhiTextureHandle TakeFramebufferTexture(RhiFramebufferHandle framebuffer) override;
	RhiTextureHandle ReallocateFramebufferTexture(RhiFramebufferHandle framebuffer, const RhiFramebufferCreateDesc& desc) override;
	void ResizeFramebufferRenderbuffer(RhiFramebufferHandle framebuffer, int width, int height) override;
	void ReadFramebufferColorPixel(RhiFramebufferHandle framebuffer, uint32_t x, uint32_t y, float* rgb) override;

	RhiMeshHandle CreateMesh(const RhiMeshDesc& desc) override;
	void UpdateMeshVertexData(RhiMeshHandle mesh, size_t offset, size_t size, const void* data) override;
	void DestroyMesh(RhiMeshHandle mesh) override;
	void DrawMesh(RhiMeshHandle mesh, RhiPrimitiveTopology topology, uint32_t index_count) override;

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
	static unsigned ToGLFramebufferTarget(RhiFramebufferBindTarget target);
	static unsigned ToGLPrimitiveTopology(RhiPrimitiveTopology topology);

private:
	struct FramebufferResources {
		std::vector<unsigned> color_textures;
		unsigned depth_texture = 0;
		unsigned render_buffer = 0;
		unsigned texture_type = 0;
		RhiFramebufferAttachment attachment = RhiFramebufferAttachment::Color;
	};
	struct MeshResources {
		unsigned vertex_array = 0;
		unsigned vertex_buffer = 0;
		unsigned index_buffer = 0;
		uint32_t index_count = 0;
		uint32_t vertex_count = 0;
	};

	std::shared_ptr<MyGL> gl_;
	std::unordered_map<uint64_t, RhiBufferUsage> buffer_usages_;
	std::unordered_map<uint64_t, FramebufferResources> framebuffer_resources_;
	std::unordered_map<uint64_t, MeshResources> mesh_resources_;
};

std::shared_ptr<OpenGLRhiDevice> AsOpenGLRhiDevice(const std::shared_ptr<IRhiDevice>& device);

} // namespace GComponent

#endif // GSIM_RENDER_RHI_OPENGL_DEVICE_H
