#include "render/framebufferobject.h"

#include <cstring>
#include <iostream>

namespace GComponent {

namespace {

bool UsesUnsupportedFramebufferAttachment(FrameBufferObject::AttachType type, const std::shared_ptr<IRhiDevice>& rhi_device)
{
	return rhi_device
		&& rhi_device->GetBackendType() == RhiBackendType::DirectX12
		&& (type == FrameBufferObject::Cube || type == FrameBufferObject::CubeMipmap);
}

void LogFramebufferObjectSkipOnce(FrameBufferObject::AttachType type)
{
	static bool logged_cube = false;
	if ((type == FrameBufferObject::Cube || type == FrameBufferObject::CubeMipmap) && !logged_cube) {
		std::cerr << "FrameBufferObject disabled: DirectX12 cubemap framebuffer attachments are not implemented yet\n";
		logged_cube = true;
	}
}

} // namespace

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
	if (!IsAvailable() || !rhi_device_) return;
	rhi_device_->BindFramebuffer(RhiFramebufferBindTarget::Framebuffer, frame_buffer_);
}

void FrameBufferObject::Release()
{
	if (!IsAvailable() || !rhi_device_) return;
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
	if (!IsAvailable() || !rhi_device_) return 0;
	const auto ret = rhi_device_->TakeFramebufferTexture(frame_buffer_);
	texture_buffer_ = {};
	return static_cast<unsigned int>(ret.value);
}

unsigned int FrameBufferObject::ReAllocateTexture(int width, int height, AttachType type)
{
	if (!IsAvailable() || !rhi_device_) return 0;
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
	if (!rhi_device_) {
		return;
	}
	if (UsesUnsupportedFramebufferAttachment(type, rhi_device_)) {
		LogFramebufferObjectSkipOnce(type);
		return;
	}

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
	if (!IsAvailable() || !rhi_device_) return;
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
