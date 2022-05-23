#include "picking_helper.h"

#include <iostream>

#include <QtGui/QOpenGLContext>

GComponent::PickingController::PickingController() = default;

GComponent::PickingController::~PickingController() = default;

void GComponent::PickingController::SetGL(const shared_ptr<MyGL>& other)
{
	gl_ = other;
}

bool GComponent::PickingController::Init(unsigned width, unsigned height)
{
	CheckHaveInit();

	gl_->glGenFramebuffers(1, &FBO_);
	gl_->glGenTextures(1, &picking_texture_obeject_);
	gl_->glGenTextures(1, &depth_texture_object_);

	gl_->glBindFramebuffer(GL_FRAMEBUFFER, FBO_);

	gl_->glBindTexture(GL_TEXTURE_2D, picking_texture_obeject_);
	gl_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, picking_texture_obeject_, 0);
	
	gl_->glBindTexture(GL_TEXTURE_2D, depth_texture_object_);
	gl_->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	gl_->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture_object_, 0);

	gl_->glReadBuffer(GL_NONE);
	gl_->glDrawBuffer(GL_COLOR_ATTACHMENT0);

	if (gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::printf("FRAME BUFFER ERROR, STATUS: 0x%x\n", gl_->glCheckFramebufferStatus(GL_FRAMEBUFFER));
		return false;
	}

	gl_->glBindTexture(GL_TEXTURE_2D, 0);
	gl_->glBindFramebuffer(GL_FRAMEBUFFER, default_FBO_);

	have_init_ = true;
	return true;
}

void GComponent::PickingController::EnablePickingMode(unsigned default_FBO)
{
	gl_->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO_);
	default_FBO_ = default_FBO;
}

void GComponent::PickingController::DisablePickintMode()
{
	gl_->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, default_FBO_);
}

GComponent::PickingPixelInfo GComponent::PickingController::GetPickingPixelInfo(unsigned u, unsigned v)
{
	gl_->glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO_);
	gl_->glReadBuffer(GL_COLOR_ATTACHMENT0);

	PickingPixelInfo info;
	gl_->glReadPixels(u, v, 1, 1, GL_RGB, GL_FLOAT, &info);

	gl_->glReadBuffer(GL_NONE);
	gl_->glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	return info;
}

void GComponent::PickingController::CheckHaveInit()
{
	if (have_init_) {
		gl_->glDeleteFramebuffers(1, &FBO_);
		gl_->glDeleteTextures(1, &picking_texture_obeject_);
		gl_->glDeleteTextures(1, &depth_texture_object_);

		FBO_ = picking_texture_obeject_ = depth_texture_object_ = 0;
		have_init_ = false;
	}
}

GComponent::PickingGuard::PickingGuard(PickingController& controller):
	controller_(controller)
{
	controller_.EnablePickingMode(QOpenGLContext::currentContext()->defaultFramebufferObject());
}

GComponent::PickingGuard::~PickingGuard()
{
	controller_.DisablePickintMode();
}
