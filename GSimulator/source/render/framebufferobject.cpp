#include "render/framebufferobject.h"

#include <QtGui/QOpenGLContext>
namespace GComponent{
std::map<FrameBufferObject::BufferType, FrameBufferObject::BufferDefaultOption> 
FrameBufferObject::option_map = {
	{FrameBufferObject::Color, {GL_RGB32F,			   GL_RGB,			   GL_LINEAR,  GL_REPEAT,		   GL_COLOR_ATTACHMENT0 }},
	{FrameBufferObject::Depth, {GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_DEPTH_ATTACHMENT}}
};

GComponent::FrameBufferObject::FrameBufferObject(int width, int height, const std::shared_ptr<MyGL>& other, BufferType type)
{
	gl_ = other;
	
	const BufferDefaultOption& opt = option_map[type];
	// gen buffers
	gl_->glGenFramebuffers(1, &frame_buffer_);
	gl_->glGenTextures(1, &texture_buffer_);

	// allocate memory for color attachment
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
	gl_->glBindTexture(GL_TEXTURE_2D, texture_buffer_);
	gl_->glTexImage2D(GL_TEXTURE_2D, 0,  opt.internal_format, width, height, 0, opt.format, GL_FLOAT, nullptr);
	gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, opt.filter);
	gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, opt.filter);
	gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     opt.wrap);
	gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     opt.wrap);
	if (opt.wrap == GL_CLAMP_TO_BORDER) {
		constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		gl_->glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, bordercolor);
	}
	gl_->glBindTexture(GL_TEXTURE_2D, 0);

	if (type == FrameBufferObject::Color) {
		// allocate memory for render attachment
		gl_->glGenRenderbuffers(1, &render_buffer_);
		gl_->glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_);
		gl_->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		gl_->glBindRenderbuffer(GL_RENDERBUFFER, 0);
		gl_->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer_);
	}
	// set attachment to frame buffer
	gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, opt.attachment, GL_TEXTURE_2D, texture_buffer_, 0);

	if (gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::printf("FRAME BUFFER INIT ERROR, STATUS: 0x%x\n", gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER));		
	}

	gl_->glBindFramebuffer(GL_FRAMEBUFFER, GetDefaultFBO());
}

GComponent::FrameBufferObject::~FrameBufferObject()
{
	Clear();
}

GComponent::FrameBufferObject::FrameBufferObject(FrameBufferObject&& other) noexcept
{
	if (&other == this) return;
	Clear();
	memmove(this, &other, sizeof type - sizeof gl_);
	memset(&other, 0,	  sizeof type - sizeof gl_);
	gl_ = std::move(other.gl_);
}

GComponent::FrameBufferObject& GComponent::FrameBufferObject::operator=(FrameBufferObject&& other) noexcept
{
	if (&other == this) return *this;
	Clear();
	memmove(this, &other, sizeof type - sizeof gl_);
	memset(&other, 0,	  sizeof type - sizeof gl_);
	gl_ = std::move(other.gl_);
	return *this;
}

void GComponent::FrameBufferObject::Clear()
{
	if (frame_buffer_) {
		gl_->glDeleteFramebuffers(1, &frame_buffer_);
	}
	if (texture_buffer_) {
		gl_->glDeleteTextures(1, &texture_buffer_);
	}
	if (render_buffer_) {
		gl_->glDeleteRenderbuffers(1, &render_buffer_);
	}
}

}