#include "render/framebufferobject.h"

#include <QtGui/QOpenGLContext>

GComponent::FrameBufferObject::FrameBufferObject(int width, int height, const std::shared_ptr<MyGL>& other)
{
	gl_ = other;

	// gen buffers
	gl_->glGenFramebuffers(1, &frame_buffer_);
	gl_->glGenTextures(1, &texture_buffer_);

	// allocate memory for color attachment
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
	gl_->glBindTexture(GL_TEXTURE_2D, texture_buffer_);
	gl_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gl_->glBindTexture(GL_TEXTURE_2D, 0);

	// allocate memory for render attachment
	gl_->glGenRenderbuffers(1, &render_buffer_);
	gl_->glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_);
	gl_->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	gl_->glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// set attachment to frame buffer
	gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_buffer_, 0);
	gl_->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer_);

	if (gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::printf("FRAME BUFFER INIT ERROR, STATUS: 0x%x\n", gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER));		
	}

	gl_->glBindFramebuffer(GL_FRAMEBUFFER, GetDefaultFBO());
}

GComponent::FrameBufferObject::~FrameBufferObject()
{
	Clear();
}

GComponent::FrameBufferObject::FrameBufferObject(FrameBufferObject&& other)
{
	if (&other == this) return;
	Clear();
	memmove(this, &other, sizeof type - sizeof gl_);
	memset(&other, 0,	  sizeof type - sizeof gl_);
	gl_ = std::move(other.gl_);
}

GComponent::FrameBufferObject& GComponent::FrameBufferObject::operator=(FrameBufferObject&& other)
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
