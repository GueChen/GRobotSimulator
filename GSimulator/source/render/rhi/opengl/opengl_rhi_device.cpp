#include "render/rhi/opengl/opengl_rhi_device.h"

#include <QtGui/QOpenGLContext>

#include <utility>

namespace GComponent {

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

unsigned OpenGLRhiDevice::ToGLCapability(RhiCapability capability)
{
	switch (capability) {
	case RhiCapability::DepthTest:
		return GL_DEPTH_TEST;
	case RhiCapability::Blend:
		return GL_BLEND;
	case RhiCapability::Multisample:
		return GL_MULTISAMPLE;
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

std::shared_ptr<OpenGLRhiDevice> AsOpenGLRhiDevice(const std::shared_ptr<IRhiDevice>& device)
{
	return std::dynamic_pointer_cast<OpenGLRhiDevice>(device);
}

std::shared_ptr<IRhiDevice> CreateOpenGLRhiDevice()
{
	return std::make_shared<OpenGLRhiDevice>();
}

} // namespace GComponent
