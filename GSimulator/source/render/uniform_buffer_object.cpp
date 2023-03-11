#include "uniform_buffer_object.h"

namespace GComponent{

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