#include "uniform_buffer_object.h"

namespace GComponent{

UniformBufferObject::UniformBufferObject(int binding, size_t size, const std::shared_ptr<IRhiDevice>& rhi_device) :
	binding_pos_(binding),
	size_(size),
	rhi_device_(rhi_device)
{
	ubo_ = rhi_device_->CreateBuffer(RhiBufferDesc{
		.usage = RhiBufferUsage::Uniform,
		.size = size_
	});
	rhi_device_->BindUniformBufferBase(static_cast<uint32_t>(binding_pos_), ubo_);
}

UniformBufferObject::~UniformBufferObject()
{
	Clear();
}

void UniformBufferObject::SetData(const void* data, size_t size)
{
	rhi_device_->UpdateBuffer(ubo_, 0, size, data);
}

void UniformBufferObject::SetSubData(const void* data, size_t offset, size_t size)
{
	rhi_device_->UpdateBuffer(ubo_, offset, size, data);
}

UniformBufferObject::UniformBufferObject(UniformBufferObject&& other) noexcept
{
	if (&other == this) return;
	Clear();
	memmove(this,   &other, sizeof UniformBufferObject - sizeof rhi_device_);
	memset (&other, 0,      sizeof UniformBufferObject - sizeof rhi_device_);
	rhi_device_ = std::move(other.rhi_device_);
}

UniformBufferObject& UniformBufferObject::operator=(UniformBufferObject&& other) noexcept
{
	if (&other == this) return *this;
	Clear();
	memmove(this, &other, sizeof UniformBufferObject - sizeof rhi_device_);
	memset(&other, 0,     sizeof UniformBufferObject - sizeof rhi_device_);
	rhi_device_ = std::move(other.rhi_device_);
	return *this;
}

void UniformBufferObject::Clear()
{
	if (ubo_ && rhi_device_) {
		rhi_device_->DestroyBuffer(ubo_);
		ubo_ = {};
	}
}

}