#include "uniform_buffer_object.h"

#include "render/rhi/opengl/opengl_rhi_device.h"

#include <stdexcept>

namespace GComponent{

static std::shared_ptr<MyGL> GetOpenGL(const std::shared_ptr<IRhiDevice>& rhi_device)
{
	auto opengl_device = AsOpenGLRhiDevice(rhi_device);
	if (!opengl_device) {
		throw std::runtime_error("UniformBufferObject currently requires an OpenGL RHI device");
	}
	return opengl_device->GetGL();
}

UniformBufferObject::UniformBufferObject(int binding, size_t size, const std::shared_ptr<MyGL>& other) :
	binding_pos_(binding),
	size_(size),
	gl_(other)
{
	gl_->glGenBuffers(1, &ubo_);
	Bind();
	gl_->glBufferData    (GL_UNIFORM_BUFFER, size_, nullptr, GL_STATIC_DRAW);
	gl_->glBindBufferBase(GL_UNIFORM_BUFFER, binding_pos_, ubo_);
	Release();
}

UniformBufferObject::UniformBufferObject(int binding, size_t size, const std::shared_ptr<IRhiDevice>& rhi_device):
	UniformBufferObject(binding, size, GetOpenGL(rhi_device))
{}

UniformBufferObject::~UniformBufferObject()
{
	Clear();
}

void UniformBufferObject::SetData(const void* data, size_t size)
{
	gl_->glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
}

void UniformBufferObject::SetSubData(const void* data, size_t offset, size_t size)
{
	gl_->glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}

UniformBufferObject::UniformBufferObject(UniformBufferObject&& other) noexcept
{
	if (&other == this) return;
	Clear();
	memmove(this,   &other, sizeof UniformBufferObject - sizeof gl_);
	memset (&other, 0,      sizeof UniformBufferObject - sizeof gl_);
	gl_ = std::move(other.gl_);
}

UniformBufferObject& UniformBufferObject::operator=(UniformBufferObject&& other) noexcept
{
	if (&other == this) return *this;
	Clear();
	memmove(this, &other, sizeof UniformBufferObject - sizeof gl_);
	memset(&other, 0,     sizeof UniformBufferObject - sizeof gl_);
	gl_ = std::move(other.gl_);
	return *this;
}

void UniformBufferObject::Clear()
{
	if (ubo_) {
		gl_->glDeleteBuffers(1, &ubo_);
	}
}

}