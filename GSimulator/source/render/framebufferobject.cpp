#include "render/framebufferobject.h"

#include <QtGui/QOpenGLContext>
namespace GComponent{
std::map<FrameBufferObject::AttachType, FrameBufferObject::BufferOption> 
FrameBufferObject::option_map = {
	{FrameBufferObject::Color,		{GL_RGB32F,			   GL_RGB,			   GL_LINEAR,				 GL_LINEAR,		GL_REPEAT,			GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0, GL_NONE}},
	{FrameBufferObject::Depth,		{GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_NEAREST,				 GL_NEAREST,	GL_CLAMP_TO_BORDER, GL_DEPTH_ATTACHMENT,  GL_NONE,			    GL_NONE}},
	{FrameBufferObject::Cube,		{GL_RGB16F,			   GL_RGB,			   GL_LINEAR,			     GL_LINEAR,		GL_CLAMP_TO_EDGE,   GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0, GL_NONE}},
	{FrameBufferObject::CubeMipmap, {GL_RGB16F,			   GL_RGB,			   GL_LINEAR_MIPMAP_LINEAR,	 GL_LINEAR,		GL_CLAMP_TO_EDGE,   GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT0, GL_NONE}}
};

GComponent::FrameBufferObject::FrameBufferObject(int width, int height, AttachType type, const std::shared_ptr<MyGL>& other)
{
	gl_ = other;
	texture_type_ = type == Color? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
	Initialize(width, height, 0, type);	

}

FrameBufferObject::FrameBufferObject(int width, int height, int level, AttachType type, const std::shared_ptr<MyGL>& other)
{
	gl_ = other;
	texture_type_ = GL_TEXTURE_2D_ARRAY;
	Initialize(width, height, level, type);	
}

GComponent::FrameBufferObject::~FrameBufferObject()
{
	Clear();
}

unsigned int FrameBufferObject::TakeTexture() {
	unsigned int ret = texture_buffer_;
	texture_buffer_ = 0;
	return ret;
}

unsigned int FrameBufferObject::ReAllocateTexture(int width, int height, AttachType type)
{
	unsigned int ret = texture_buffer_;
	texture_type_ = type == Color ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
	const BufferOption& opt = option_map[type];

	AdjustRenderBufferStorage(width, height);

	GenTexture(width, height, 0, opt);		

	//BindTextureOnFrameBuffer(opt);
	return ret;
}

void FrameBufferObject::Initialize(int width, int height, int levels, AttachType type)
{
	const BufferOption& opt = option_map[type];
	gl_->glGenFramebuffers(1, &frame_buffer_);

	GenTexture(width, height, levels, opt);

	if (type != FrameBufferObject::Depth) {
		GenRenderBuffer(width, height);
	}

	BindTextureOnFrameBuffer(opt);

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

void FrameBufferObject::GenTexture(int width, int height, int level, const BufferOption& opt)
{
	gl_->glGenTextures(1, &texture_buffer_);

	gl_->glBindTexture(texture_type_, texture_buffer_);
	switch (texture_type_) {
	case GL_TEXTURE_2D:
		gl_->glTexImage2D(texture_type_, 0, opt.internal_format, width, height, 0, opt.format, GL_FLOAT, nullptr);
		break;
	case GL_TEXTURE_2D_ARRAY:
		gl_->glTexImage3D(texture_type_, 0, opt.internal_format, width, height, level, 0, opt.format, GL_FLOAT, nullptr);
		break;
	case GL_TEXTURE_CUBE_MAP: {
		for (uint32_t i = 0; i < 6; ++i) {
			gl_->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, opt.internal_format, width, height, 0, opt.format, GL_FLOAT, nullptr);
		}
	}
	}
	gl_->glTexParameteri(texture_type_, GL_TEXTURE_MIN_FILTER, opt.min_filter);
	gl_->glTexParameteri(texture_type_, GL_TEXTURE_MAG_FILTER, opt.mag_filter);
	gl_->glTexParameteri(texture_type_, GL_TEXTURE_WRAP_S, opt.wrap);
	gl_->glTexParameteri(texture_type_, GL_TEXTURE_WRAP_T, opt.wrap);
	if (texture_type_ == GL_TEXTURE_CUBE_MAP) {
		gl_->glTexParameteri(texture_type_, GL_TEXTURE_WRAP_R, opt.wrap);
	}

	if (opt.wrap == GL_CLAMP_TO_BORDER) {
		constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		gl_->glTexParameterfv(texture_type_, GL_TEXTURE_BORDER_COLOR, bordercolor);
	}

	gl_->glBindTexture(texture_type_, 0);
}

void FrameBufferObject::GenRenderBuffer(int width, int height)
{
	gl_->glGenRenderbuffers(1, &render_buffer_);
	gl_->glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_);
	gl_->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	
	gl_->glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FrameBufferObject::BindTextureOnFrameBuffer(const BufferOption& opt)
{
	// binding buffer to frame buffer	
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_);
	
	if (texture_type_ != GL_TEXTURE_CUBE_MAP) {
		gl_->glFramebufferTexture(GL_FRAMEBUFFER, opt.attachment, texture_buffer_, 0);
		gl_->glDrawBuffer(opt.draw_buffer);
		gl_->glReadBuffer(opt.read_buffer);
	}

	if (render_buffer_)
	{
		gl_->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer_);
	}

	if (gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::printf("FRAME BUFFER INIT ERROR, STATUS: 0x%x\n", gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER));
	}

	gl_->glBindFramebuffer(GL_FRAMEBUFFER, GetDefaultFBO());
}

void FrameBufferObject::AdjustRenderBufferStorage(int width, int height) {
	gl_->glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_);
	gl_->glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	gl_->glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

}