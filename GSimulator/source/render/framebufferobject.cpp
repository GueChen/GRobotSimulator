#include "render/framebufferobject.h"

#include <cstring>

namespace GComponent {

FrameBufferObject::FrameBufferObject(int width, int height, AttachType type, const std::shared_ptr<IRhiDevice>& rhi_device):
	rhi_device_(rhi_device)
{
	Initialize(width, height, 0, type);
}

FrameBufferObject::FrameBufferObject(int width, int height, int level, AttachType type, const std::shared_ptr<IRhiDevice>& rhi_device):
	rhi_device_(rhi_device)
{
	Initialize(width, height, level, type);
}

FrameBufferObject::~FrameBufferObject()
{
	Clear();
}

void FrameBufferObject::Bind()
{
	rhi_device_->BindFramebuffer(RhiFramebufferBindTarget::Framebuffer, frame_buffer_);
}

void FrameBufferObject::Release()
{
	rhi_device_->BindDefaultFramebuffer(RhiFramebufferBindTarget::Framebuffer);
}

void FrameBufferObject::BindTexture(uint32_t texture_pos)
{
	texture_pos_ = texture_pos >= 0x84C0 ? texture_pos - 0x84C0 : texture_pos;
	rhi_device_->BindTextureUnit(texture_pos_, texture_buffer_);
}

void FrameBufferObject::ReleaseTexture()
{
	rhi_device_->BindTextureUnit(texture_pos_, {});
}

unsigned int FrameBufferObject::TakeTexture()
{
	const auto ret = rhi_device_->TakeFramebufferTexture(frame_buffer_);
	texture_buffer_ = {};
	return static_cast<unsigned int>(ret.value);
}

unsigned int FrameBufferObject::ReAllocateTexture(int width, int height, AttachType type)
{
	const auto ret = rhi_device_->ReallocateFramebufferTexture(frame_buffer_, RhiFramebufferCreateDesc{
		.width = width,
		.height = height,
		.attachment = ToRhiAttachment(type)
	});
	texture_buffer_ = rhi_device_->GetFramebufferTexture(frame_buffer_);
	return static_cast<unsigned int>(ret.value);
}

void FrameBufferObject::Initialize(int width, int height, int levels, AttachType type)
{
	frame_buffer_ = rhi_device_->CreateFramebuffer(RhiFramebufferCreateDesc{
		.width = width,
		.height = height,
		.layers = levels,
		.attachment = ToRhiAttachment(type)
	});
	texture_buffer_ = rhi_device_->GetFramebufferTexture(frame_buffer_);
}

FrameBufferObject::FrameBufferObject(FrameBufferObject&& other) noexcept
{
	if (&other == this) return;
	Clear();
	memmove(this, &other, sizeof type - sizeof rhi_device_);
	memset(&other, 0, sizeof type - sizeof rhi_device_);
	rhi_device_ = std::move(other.rhi_device_);
}

FrameBufferObject& FrameBufferObject::operator=(FrameBufferObject&& other) noexcept
{
	if (&other == this) return *this;
	Clear();
	memmove(this, &other, sizeof type - sizeof rhi_device_);
	memset(&other, 0, sizeof type - sizeof rhi_device_);
	rhi_device_ = std::move(other.rhi_device_);
	return *this;
}

void FrameBufferObject::Clear()
{
	if (frame_buffer_ && rhi_device_) {
		rhi_device_->DestroyFramebuffer(frame_buffer_);
		frame_buffer_ = {};
		texture_buffer_ = {};
	}
}

void FrameBufferObject::AdjustRenderBufferStorage(int width, int height)
{
	rhi_device_->ResizeFramebufferRenderbuffer(frame_buffer_, width, height);
}

RhiFramebufferAttachment FrameBufferObject::ToRhiAttachment(AttachType type)
{
	switch (type) {
	case Color:
		return RhiFramebufferAttachment::Color;
	case Depth:
		return RhiFramebufferAttachment::Depth;
	case Cube:
		return RhiFramebufferAttachment::Cube;
	case CubeMipmap:
		return RhiFramebufferAttachment::CubeMipmap;
	}
	return RhiFramebufferAttachment::Color;
}

} // namespace GComponent
