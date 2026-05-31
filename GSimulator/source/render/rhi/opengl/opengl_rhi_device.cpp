#include "render/rhi/opengl/opengl_rhi_device.h"

#include <QtGui/QOpenGLContext>

#include <cstdio>
#include <string>
#include <utility>

namespace GComponent {

namespace {

struct OpenGLFramebufferOption {
	unsigned internal_format = GL_RGB32F;
	unsigned format = GL_RGB;
	unsigned min_filter = GL_LINEAR;
	unsigned mag_filter = GL_LINEAR;
	unsigned wrap = GL_REPEAT;
	unsigned attachment = GL_COLOR_ATTACHMENT0;
	unsigned draw_buffer = GL_COLOR_ATTACHMENT0;
	unsigned read_buffer = GL_NONE;
};

OpenGLFramebufferOption GetFramebufferOption(RhiFramebufferAttachment attachment)
{
	switch (attachment) {
	case RhiFramebufferAttachment::Color:
		return { GL_RGB32F, GL_RGB, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0, GL_NONE };
	case RhiFramebufferAttachment::Depth:
		return { GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_DEPTH_ATTACHMENT, GL_NONE, GL_NONE };
	case RhiFramebufferAttachment::Cube:
		return { GL_RGB16F, GL_RGB, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0, GL_NONE };
	case RhiFramebufferAttachment::CubeMipmap:
		return { GL_RGB16F, GL_RGB, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0, GL_NONE };
	}
	return {};
}

unsigned GetFramebufferTextureType(const RhiFramebufferCreateDesc& desc)
{
	if (desc.layers > 0) {
		return GL_TEXTURE_2D_ARRAY;
	}
	return desc.attachment == RhiFramebufferAttachment::Color ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
}

} // namespace

OpenGLRhiDevice::OpenGLRhiDevice():
	gl_(std::make_shared<MyGL>())
{}

OpenGLRhiDevice::OpenGLRhiDevice(std::shared_ptr<MyGL> gl):
	gl_(std::move(gl))
{}

RhiBackendType OpenGLRhiDevice::GetBackendType() const
{
	return RhiBackendType::OpenGL;
}

void OpenGLRhiDevice::Initialize()
{
	gl_->initializeOpenGLFunctions();
}

void OpenGLRhiDevice::Enable(RhiCapability capability)
{
	gl_->glEnable(ToGLCapability(capability));
}

void OpenGLRhiDevice::Disable(RhiCapability capability)
{
	gl_->glDisable(ToGLCapability(capability));
}

void OpenGLRhiDevice::SetDepthFunc(RhiDepthFunc func)
{
	gl_->glDepthFunc(ToGLDepthFunc(func));
}

void OpenGLRhiDevice::SetCullFace(RhiCullFace face)
{
	gl_->glCullFace(ToGLCullFace(face));
}

void OpenGLRhiDevice::SetPolygonMode(RhiPolygonMode mode)
{
	gl_->glPolygonMode(GL_FRONT_AND_BACK, ToGLPolygonMode(mode));
}

void OpenGLRhiDevice::SetBlendAlpha()
{
	gl_->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void OpenGLRhiDevice::SetLineWidth(float width)
{
	gl_->glLineWidth(width);
}

void OpenGLRhiDevice::SetViewport(const RhiViewport& viewport)
{
	gl_->glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
}

void OpenGLRhiDevice::SetClearColor(const RhiClearColor& color)
{
	gl_->glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRhiDevice::Clear(RhiClearFlags flags)
{
	gl_->glClear(ToGLClearFlags(flags));
}

void OpenGLRhiDevice::BindTextureUnit(uint32_t unit, RhiTextureHandle texture)
{
	gl_->glBindTextureUnit(unit, static_cast<unsigned>(texture.value));
}

void OpenGLRhiDevice::BindDefaultFramebuffer()
{
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, GetDefaultFramebuffer());
}

void OpenGLRhiDevice::BindDefaultFramebuffer(RhiFramebufferBindTarget target)
{
	gl_->glBindFramebuffer(ToGLFramebufferTarget(target), GetDefaultFramebuffer());
}

void OpenGLRhiDevice::BindFramebuffer(RhiFramebufferBindTarget target, RhiFramebufferHandle framebuffer)
{
	gl_->glBindFramebuffer(ToGLFramebufferTarget(target), static_cast<unsigned>(framebuffer.value));
}

uint32_t OpenGLRhiDevice::GetDefaultFramebuffer() const
{
	return QOpenGLContext::currentContext()->defaultFramebufferObject();
}

RhiBufferHandle OpenGLRhiDevice::CreateBuffer(const RhiBufferDesc& desc, const void* initial_data)
{
	unsigned buffer = 0;
	const unsigned target = ToGLBufferTarget(desc.usage);
	gl_->glGenBuffers(1, &buffer);
	gl_->glBindBuffer(target, buffer);
	gl_->glBufferData(target, desc.size, initial_data, GL_STATIC_DRAW);
	gl_->glBindBuffer(target, 0);
	buffer_usages_.emplace(buffer, desc.usage);
	return RhiBufferHandle{ buffer };
}

void OpenGLRhiDevice::BindBuffer(RhiBufferUsage usage, RhiBufferHandle buffer)
{
	gl_->glBindBuffer(ToGLBufferTarget(usage), static_cast<unsigned>(buffer.value));
}

void OpenGLRhiDevice::BindUniformBufferBase(uint32_t binding, RhiBufferHandle buffer)
{
	gl_->glBindBufferBase(GL_UNIFORM_BUFFER, binding, static_cast<unsigned>(buffer.value));
}

void OpenGLRhiDevice::UpdateBuffer(RhiBufferHandle buffer, size_t offset, size_t size, const void* data)
{
	const auto iter = buffer_usages_.find(buffer.value);
	const unsigned target = iter == buffer_usages_.end()
		? GL_ARRAY_BUFFER
		: ToGLBufferTarget(iter->second);
	gl_->glBindBuffer(target, static_cast<unsigned>(buffer.value));
	gl_->glBufferSubData(target, offset, size, data);
	gl_->glBindBuffer(target, 0);
}

void OpenGLRhiDevice::DestroyBuffer(RhiBufferHandle buffer)
{
	unsigned gl_buffer = static_cast<unsigned>(buffer.value);
	if (gl_buffer) {
		gl_->glDeleteBuffers(1, &gl_buffer);
		buffer_usages_.erase(buffer.value);
	}
}

RhiTextureHandle OpenGLRhiDevice::CreateTexture(const RhiTextureDesc& desc, const void* initial_data)
{
	unsigned texture = 0;
	const unsigned target = ToGLTextureTarget(desc.dimension);
	const unsigned internal_format = ToGLInternalFormat(desc.format);
	const unsigned format = ToGLFormat(desc.format);
	const unsigned type = ToGLType(desc.format);
	gl_->glGenTextures(1, &texture);
	gl_->glBindTexture(target, texture);
	switch (desc.dimension) {
	case RhiTextureDimension::Texture2D:
		gl_->glTexImage2D(target, 0, internal_format, desc.width, desc.height, 0, format, type, initial_data);
		break;
	case RhiTextureDimension::Texture2DArray:
		gl_->glTexImage3D(target, 0, internal_format, desc.width, desc.height, desc.layers, 0, format, type, initial_data);
		break;
	case RhiTextureDimension::TextureCube:
		for (int i = 0; i < 6; ++i) {
			gl_->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, desc.width, desc.height, 0, format, type, nullptr);
		}
		break;
	}
	gl_->glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl_->glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl_->glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	gl_->glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (desc.dimension == RhiTextureDimension::TextureCube) {
		gl_->glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}
	gl_->glBindTexture(target, 0);
	return RhiTextureHandle{ texture };
}

void OpenGLRhiDevice::DestroyTexture(RhiTextureHandle texture)
{
	unsigned gl_texture = static_cast<unsigned>(texture.value);
	if (gl_texture) {
		gl_->glDeleteTextures(1, &gl_texture);
	}
}

RhiTextureHandle OpenGLRhiDevice::LoadTexture2D(std::string_view path, bool repeat)
{
	return RhiTextureHandle{ gl_->LoadTexture(std::string(path), repeat) };
}

RhiTextureHandle OpenGLRhiDevice::LoadCubemap(const std::vector<std::string_view>& paths)
{
	std::vector<std::string> owned_paths;
	owned_paths.reserve(paths.size());
	for (const auto path : paths) {
		owned_paths.emplace_back(path);
	}
	return RhiTextureHandle{ gl_->LoadCubemap(owned_paths) };
}

RhiFramebufferHandle OpenGLRhiDevice::CreatePickingFramebuffer(int width, int height)
{
	unsigned framebuffer = 0;
	unsigned picking_texture = 0;
	unsigned depth_texture = 0;

	gl_->glGenFramebuffers(1, &framebuffer);
	gl_->glGenTextures(1, &picking_texture);
	gl_->glGenTextures(1, &depth_texture);

	gl_->glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	gl_->glBindTexture(GL_TEXTURE_2D, picking_texture);
	gl_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, picking_texture, 0);

	gl_->glBindTexture(GL_TEXTURE_2D, depth_texture);
	gl_->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture, 0);

	gl_->glReadBuffer(GL_NONE);
	gl_->glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::printf("PICKING FRAME BUFFER ERROR, STATUS: 0x%x\n", gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER));
	}

	gl_->glBindTexture(GL_TEXTURE_2D, 0);
	BindDefaultFramebuffer();

	framebuffer_resources_.emplace(framebuffer, FramebufferResources{
		.color_texture = picking_texture,
		.depth_texture = depth_texture
	});
	return RhiFramebufferHandle{ framebuffer };
}

RhiFramebufferHandle OpenGLRhiDevice::CreateFramebuffer(const RhiFramebufferCreateDesc& desc)
{
	const OpenGLFramebufferOption opt = GetFramebufferOption(desc.attachment);
	const unsigned texture_type = GetFramebufferTextureType(desc);
	unsigned framebuffer = 0;
	unsigned texture = 0;
	unsigned render_buffer = 0;

	gl_->glGenFramebuffers(1, &framebuffer);

	gl_->glGenTextures(1, &texture);
	gl_->glBindTexture(texture_type, texture);
	switch (texture_type) {
	case GL_TEXTURE_2D:
		gl_->glTexImage2D(texture_type, 0, opt.internal_format, desc.width, desc.height, 0, opt.format, GL_FLOAT, nullptr);
		break;
	case GL_TEXTURE_2D_ARRAY:
		gl_->glTexImage3D(texture_type, 0, opt.internal_format, desc.width, desc.height, desc.layers, 0, opt.format, GL_FLOAT, nullptr);
		break;
	case GL_TEXTURE_CUBE_MAP:
		for (uint32_t i = 0; i < 6; ++i) {
			gl_->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, opt.internal_format, desc.width, desc.height, 0, opt.format, GL_FLOAT, nullptr);
		}
		break;
	}
	gl_->glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, opt.min_filter);
	gl_->glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, opt.mag_filter);
	gl_->glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, opt.wrap);
	gl_->glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, opt.wrap);
	if (texture_type == GL_TEXTURE_CUBE_MAP) {
		gl_->glTexParameteri(texture_type, GL_TEXTURE_WRAP_R, opt.wrap);
	}
	if (opt.wrap == GL_CLAMP_TO_BORDER) {
		constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		gl_->glTexParameterfv(texture_type, GL_TEXTURE_BORDER_COLOR, bordercolor);
	}
	gl_->glBindTexture(texture_type, 0);

	if (desc.attachment != RhiFramebufferAttachment::Depth) {
		gl_->glGenRenderbuffers(1, &render_buffer);
		gl_->glBindRenderbuffer(GL_RENDERBUFFER, render_buffer);
		gl_->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, desc.width, desc.height);
		gl_->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	gl_->glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	if (texture_type != GL_TEXTURE_CUBE_MAP) {
		gl_->glFramebufferTexture(GL_FRAMEBUFFER, opt.attachment, texture, 0);
		gl_->glDrawBuffer(opt.draw_buffer);
		gl_->glReadBuffer(opt.read_buffer);
	}
	if (render_buffer) {
		gl_->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer);
	}
	if (gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::printf("FRAME BUFFER INIT ERROR, STATUS: 0x%x\n", gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER));
	}
	BindDefaultFramebuffer();

	framebuffer_resources_.emplace(framebuffer, FramebufferResources{
		.color_texture = texture,
		.render_buffer = render_buffer,
		.texture_type = texture_type,
		.attachment = desc.attachment
	});
	return RhiFramebufferHandle{ framebuffer };
}

void OpenGLRhiDevice::DestroyFramebuffer(RhiFramebufferHandle framebuffer)
{
	unsigned gl_framebuffer = static_cast<unsigned>(framebuffer.value);
	if (!gl_framebuffer) {
		return;
	}

	const auto iter = framebuffer_resources_.find(framebuffer.value);
	if (iter != framebuffer_resources_.end()) {
		auto resources = iter->second;
		if (resources.color_texture) {
			gl_->glDeleteTextures(1, &resources.color_texture);
		}
		if (resources.depth_texture) {
			gl_->glDeleteTextures(1, &resources.depth_texture);
		}
		if (resources.render_buffer) {
			gl_->glDeleteRenderbuffers(1, &resources.render_buffer);
		}
		framebuffer_resources_.erase(iter);
	}

	gl_->glDeleteFramebuffers(1, &gl_framebuffer);
}

RhiTextureHandle OpenGLRhiDevice::GetFramebufferTexture(RhiFramebufferHandle framebuffer) const
{
	const auto iter = framebuffer_resources_.find(framebuffer.value);
	return iter == framebuffer_resources_.end()
		? RhiTextureHandle{}
		: RhiTextureHandle{ iter->second.color_texture };
}

RhiTextureHandle OpenGLRhiDevice::TakeFramebufferTexture(RhiFramebufferHandle framebuffer)
{
	const auto iter = framebuffer_resources_.find(framebuffer.value);
	if (iter == framebuffer_resources_.end()) {
		return {};
	}
	RhiTextureHandle texture{ iter->second.color_texture };
	iter->second.color_texture = 0;
	return texture;
}

RhiTextureHandle OpenGLRhiDevice::ReallocateFramebufferTexture(RhiFramebufferHandle framebuffer, const RhiFramebufferCreateDesc& desc)
{
	const auto iter = framebuffer_resources_.find(framebuffer.value);
	if (iter == framebuffer_resources_.end()) {
		return {};
	}

	RhiTextureHandle old_texture{ iter->second.color_texture };
	const auto replacement_framebuffer = CreateFramebuffer(desc);
	const auto replacement_iter = framebuffer_resources_.find(replacement_framebuffer.value);
	if (replacement_iter == framebuffer_resources_.end()) {
		return old_texture;
	}

	auto replacement_resources = replacement_iter->second;
	replacement_iter->second.color_texture = 0;
	DestroyFramebuffer(replacement_framebuffer);
	iter->second.color_texture = replacement_resources.color_texture;
	iter->second.texture_type = replacement_resources.texture_type;
	iter->second.attachment = replacement_resources.attachment;

	const OpenGLFramebufferOption opt = GetFramebufferOption(desc.attachment);
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, static_cast<unsigned>(framebuffer.value));
	if (replacement_resources.texture_type != GL_TEXTURE_CUBE_MAP) {
		gl_->glFramebufferTexture(GL_FRAMEBUFFER, opt.attachment, replacement_resources.color_texture, 0);
	}
	BindDefaultFramebuffer();
	return old_texture;
}

void OpenGLRhiDevice::ResizeFramebufferRenderbuffer(RhiFramebufferHandle framebuffer, int width, int height)
{
	const auto iter = framebuffer_resources_.find(framebuffer.value);
	if (iter == framebuffer_resources_.end() || !iter->second.render_buffer) {
		return;
	}
	gl_->glBindRenderbuffer(GL_RENDERBUFFER, iter->second.render_buffer);
	gl_->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	gl_->glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void OpenGLRhiDevice::ReadFramebufferColorPixel(RhiFramebufferHandle framebuffer, uint32_t x, uint32_t y, float* rgb)
{
	BindFramebuffer(RhiFramebufferBindTarget::Read, framebuffer);
	gl_->glReadBuffer(GL_COLOR_ATTACHMENT0);
	gl_->glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, rgb);
	gl_->glReadBuffer(GL_NONE);
	BindDefaultFramebuffer(RhiFramebufferBindTarget::Read);
}

RhiMeshHandle OpenGLRhiDevice::CreateMesh(const RhiMeshDesc& desc)
{
	unsigned vertex_array = 0;
	unsigned vertex_buffer = 0;
	unsigned index_buffer = 0;

	gl_->glGenVertexArrays(1, &vertex_array);
	gl_->glBindVertexArray(vertex_array);

	gl_->glGenBuffers(1, &vertex_buffer);
	gl_->glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	gl_->glBufferData(GL_ARRAY_BUFFER, desc.vertex_data_size, desc.vertex_data, GL_STATIC_DRAW);

	if (desc.index_data && desc.index_data_size > 0) {
		gl_->glGenBuffers(1, &index_buffer);
		gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
		gl_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc.index_data_size, desc.index_data, GL_STATIC_DRAW);
	}

	switch (desc.vertex_layout) {
	case RhiVertexLayout::Position3:
		gl_->glEnableVertexAttribArray(0);
		gl_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<int>(desc.vertex_stride), nullptr);
		break;
	case RhiVertexLayout::PositionNormalTexcoord:
		gl_->glEnableVertexAttribArray(0);
		gl_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<int>(desc.vertex_stride), nullptr);
		gl_->glEnableVertexAttribArray(1);
		gl_->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<int>(desc.vertex_stride), reinterpret_cast<void*>(sizeof(float) * 3));
		gl_->glEnableVertexAttribArray(2);
		gl_->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, static_cast<int>(desc.vertex_stride), reinterpret_cast<void*>(sizeof(float) * 6));
		break;
	case RhiVertexLayout::PositionNormalTexcoordColor:
		gl_->glEnableVertexAttribArray(0);
		gl_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<int>(desc.vertex_stride), nullptr);
		gl_->glEnableVertexAttribArray(1);
		gl_->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<int>(desc.vertex_stride), reinterpret_cast<void*>(sizeof(float) * 3));
		gl_->glEnableVertexAttribArray(2);
		gl_->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, static_cast<int>(desc.vertex_stride), reinterpret_cast<void*>(sizeof(float) * 6));
		gl_->glEnableVertexAttribArray(3);
		gl_->glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, static_cast<int>(desc.vertex_stride), reinterpret_cast<void*>(sizeof(float) * 8));
		break;
	}

	gl_->glBindVertexArray(0);
	gl_->glBindBuffer(GL_ARRAY_BUFFER, 0);
	if (index_buffer == 0) {
		gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	mesh_resources_.emplace(vertex_array, MeshResources{
		.vertex_array = vertex_array,
		.vertex_buffer = vertex_buffer,
		.index_buffer = index_buffer,
		.index_count = static_cast<uint32_t>(desc.index_count),
		.vertex_count = static_cast<uint32_t>(desc.vertex_count)
	});
	return RhiMeshHandle{ vertex_array };
}

void OpenGLRhiDevice::UpdateMeshVertexData(RhiMeshHandle mesh, size_t offset, size_t size, const void* data)
{
	const auto iter = mesh_resources_.find(mesh.value);
	if (iter == mesh_resources_.end()) {
		return;
	}

	gl_->glBindBuffer(GL_ARRAY_BUFFER, iter->second.vertex_buffer);
	gl_->glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	gl_->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void OpenGLRhiDevice::DestroyMesh(RhiMeshHandle mesh)
{
	const auto iter = mesh_resources_.find(mesh.value);
	if (iter == mesh_resources_.end()) {
		return;
	}

	auto resources = iter->second;
	if (resources.index_buffer) {
		gl_->glDeleteBuffers(1, &resources.index_buffer);
	}
	if (resources.vertex_buffer) {
		gl_->glDeleteBuffers(1, &resources.vertex_buffer);
	}
	if (resources.vertex_array) {
		gl_->glDeleteVertexArrays(1, &resources.vertex_array);
	}
	mesh_resources_.erase(iter);
}

void OpenGLRhiDevice::DrawMesh(RhiMeshHandle mesh, RhiPrimitiveTopology topology, uint32_t index_count)
{
	const auto iter = mesh_resources_.find(mesh.value);
	if (iter == mesh_resources_.end()) {
		return;
	}

	const auto& resources = iter->second;
	gl_->glBindVertexArray(resources.vertex_array);
	if (resources.index_buffer) {
		gl_->glDrawElements(
			ToGLPrimitiveTopology(topology),
			index_count != 0 ? index_count : resources.index_count,
			GL_UNSIGNED_INT,
			nullptr);
	}
	else {
		gl_->glDrawArrays(
			ToGLPrimitiveTopology(topology),
			0,
			index_count != 0 ? index_count : resources.vertex_count);
	}
	gl_->glBindVertexArray(0);
}

unsigned OpenGLRhiDevice::ToGLCapability(RhiCapability capability)
{
	switch (capability) {
	case RhiCapability::DepthTest:
		return GL_DEPTH_TEST;
	case RhiCapability::Blend:
		return GL_BLEND;
	case RhiCapability::Multisample:
		return GL_MULTISAMPLE;
	case RhiCapability::CullFace:
		return GL_CULL_FACE;
	}
	return GL_DEPTH_TEST;
}

unsigned OpenGLRhiDevice::ToGLDepthFunc(RhiDepthFunc func)
{
	switch (func) {
	case RhiDepthFunc::Less:
		return GL_LESS;
	case RhiDepthFunc::LessEqual:
		return GL_LEQUAL;
	}
	return GL_LESS;
}

unsigned OpenGLRhiDevice::ToGLCullFace(RhiCullFace face)
{
	switch (face) {
	case RhiCullFace::Front:
		return GL_FRONT;
	case RhiCullFace::Back:
		return GL_BACK;
	}
	return GL_BACK;
}

unsigned OpenGLRhiDevice::ToGLPolygonMode(RhiPolygonMode mode)
{
	switch (mode) {
	case RhiPolygonMode::Fill:
		return GL_FILL;
	case RhiPolygonMode::Line:
		return GL_LINE;
	}
	return GL_FILL;
}

unsigned OpenGLRhiDevice::ToGLClearFlags(RhiClearFlags flags)
{
	unsigned gl_flags = 0;
	if (HasFlag(flags, RhiClearFlags::Color)) {
		gl_flags |= GL_COLOR_BUFFER_BIT;
	}
	if (HasFlag(flags, RhiClearFlags::Depth)) {
		gl_flags |= GL_DEPTH_BUFFER_BIT;
	}
	return gl_flags;
}

unsigned OpenGLRhiDevice::ToGLBufferTarget(RhiBufferUsage usage)
{
	switch (usage) {
	case RhiBufferUsage::Vertex:
		return GL_ARRAY_BUFFER;
	case RhiBufferUsage::Index:
		return GL_ELEMENT_ARRAY_BUFFER;
	case RhiBufferUsage::Uniform:
		return GL_UNIFORM_BUFFER;
	}
	return GL_ARRAY_BUFFER;
}

unsigned OpenGLRhiDevice::ToGLTextureTarget(RhiTextureDimension dimension)
{
	switch (dimension) {
	case RhiTextureDimension::Texture2D:
		return GL_TEXTURE_2D;
	case RhiTextureDimension::Texture2DArray:
		return GL_TEXTURE_2D_ARRAY;
	case RhiTextureDimension::TextureCube:
		return GL_TEXTURE_CUBE_MAP;
	}
	return GL_TEXTURE_2D;
}

unsigned OpenGLRhiDevice::ToGLInternalFormat(RhiTextureFormat format)
{
	switch (format) {
	case RhiTextureFormat::Rgb32Float:
		return GL_RGB32F;
	case RhiTextureFormat::Rgb16Float:
		return GL_RGB16F;
	case RhiTextureFormat::Rg16Float:
		return GL_RG16F;
	case RhiTextureFormat::Depth32Float:
		return GL_DEPTH_COMPONENT32F;
	case RhiTextureFormat::Depth24Stencil8:
		return GL_DEPTH24_STENCIL8;
	}
	return GL_RGB32F;
}

unsigned OpenGLRhiDevice::ToGLFormat(RhiTextureFormat format)
{
	switch (format) {
	case RhiTextureFormat::Rgb32Float:
	case RhiTextureFormat::Rgb16Float:
		return GL_RGB;
	case RhiTextureFormat::Rg16Float:
		return GL_RG;
	case RhiTextureFormat::Depth32Float:
		return GL_DEPTH_COMPONENT;
	case RhiTextureFormat::Depth24Stencil8:
		return GL_DEPTH_STENCIL;
	}
	return GL_RGB;
}

unsigned OpenGLRhiDevice::ToGLType(RhiTextureFormat format)
{
	switch (format) {
	case RhiTextureFormat::Depth24Stencil8:
		return GL_UNSIGNED_INT_24_8;
	default:
		return GL_FLOAT;
	}
}

unsigned OpenGLRhiDevice::ToGLFramebufferTarget(RhiFramebufferBindTarget target)
{
	switch (target) {
	case RhiFramebufferBindTarget::Framebuffer:
		return GL_FRAMEBUFFER;
	case RhiFramebufferBindTarget::Draw:
		return GL_DRAW_FRAMEBUFFER;
	case RhiFramebufferBindTarget::Read:
		return GL_READ_FRAMEBUFFER;
	}
	return GL_FRAMEBUFFER;
}

unsigned OpenGLRhiDevice::ToGLPrimitiveTopology(RhiPrimitiveTopology topology)
{
	switch (topology) {
	case RhiPrimitiveTopology::Points:
		return GL_POINTS;
	case RhiPrimitiveTopology::Lines:
		return GL_LINES;
	case RhiPrimitiveTopology::LineStrip:
		return GL_LINE_STRIP;
	case RhiPrimitiveTopology::Triangles:
		return GL_TRIANGLES;
	}
	return GL_TRIANGLES;
}

std::shared_ptr<OpenGLRhiDevice> AsOpenGLRhiDevice(const std::shared_ptr<IRhiDevice>& device)
{
	return std::dynamic_pointer_cast<OpenGLRhiDevice>(device);
}

std::shared_ptr<IRhiDevice> CreateOpenGLRhiDevice()
{
	return std::make_shared<OpenGLRhiDevice>();
}

} // namespace GComponent
