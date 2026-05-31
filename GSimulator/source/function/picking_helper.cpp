#include "picking_helper.h"

#include <iostream>
#include <stdexcept>

GComponent::PickingController::PickingController()  = default;

GComponent::PickingController::PickingController(const PickingController& other):
	have_init_(other.have_init_),
	owns_framebuffer_(false),
	rhi_device_(other.rhi_device_),
	render_FBO_(other.render_FBO_)
{}

GComponent::PickingController& GComponent::PickingController::operator=(const PickingController& other)
{
	if (this == &other) {
		return *this;
	}

	CheckHaveInit();
	have_init_ = other.have_init_;
	owns_framebuffer_ = false;
	rhi_device_ = other.rhi_device_;
	render_FBO_ = other.render_FBO_;
	return *this;
}

GComponent::PickingController::~PickingController()
{
	CheckHaveInit();
}

void GComponent::PickingController::SetRhiDevice(const shared_ptr<IRhiDevice>& device)
{
	rhi_device_ = device;
}

bool GComponent::PickingController::Init(unsigned width, unsigned height)
{
	CheckHaveInit();
	if (!rhi_device_) {
		throw std::runtime_error("PickingController requires an RHI device before initialization");
	}

	render_FBO_ = rhi_device_->CreatePickingFramebuffer(static_cast<int>(width), static_cast<int>(height));
	have_init_ = true;
	return render_FBO_.IsValid();
}

void GComponent::PickingController::EnablePickingMode()
{
	rhi_device_->BindFramebuffer(RhiFramebufferBindTarget::Draw, render_FBO_);
}

void GComponent::PickingController::DisablePickintMode()
{
	rhi_device_->BindDefaultFramebuffer(RhiFramebufferBindTarget::Draw);
}

GComponent::PickingPixelInfo GComponent::PickingController::GetPickingPixelInfo(unsigned u, unsigned v)
{
	PickingPixelInfo info;
	float rgb[3] = {};
	rhi_device_->ReadFramebufferColorPixel(render_FBO_, u, v, rgb);
	info.drawID = rgb[0];
	info.modelID = rgb[1];
	info.primitiveID = rgb[2];

	return info;
}

void GComponent::PickingController::CheckHaveInit()
{
	if (have_init_) {
		if (owns_framebuffer_) {
			rhi_device_->DestroyFramebuffer(render_FBO_);
		}
		render_FBO_ = {};
		have_init_ = false;
	}
}

GComponent::PickingGuard::PickingGuard(PickingController& controller):
	controller_(controller)
{
	controller_.EnablePickingMode();
}

GComponent::PickingGuard::~PickingGuard()
{
	controller_.DisablePickintMode();
}
