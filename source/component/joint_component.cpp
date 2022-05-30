#include "component/joint_component.h"

#include "model/model.h"
#include "function/conversion.hpp"

#include <iostream>
#include <format>

namespace GComponent{

JointComponent::JointComponent(Model* ptr_parent, Vec3 bind_axis, _OptDelFun del_fun):
	Component(ptr_parent), axis_(bind_axis), opt_del_func_(del_fun)
{
	switch (type_) {
		using enum JointType;
	case Revolute: {
		zero_pos_ = ptr_parent_->getRotLocal();
		break;
	}
	case Prismatic: {
		zero_pos_ = ptr_parent_->getTransLocal();
		break;
	}
	}
}

void JointComponent::tick(float delta_time)
{
	switch (mode_) {
	using enum JointMode;
	case Position: {
		if (!pos_buffers_.empty()) {
			pos_ = pos_buffers_.front();
			pos_buffers_.pop_front();			
		}		
		break;
	}
	case Velocity: {
		// TODO: add vel mode
		if (!vel_buffers_.empty()) {
			vel_ = pos_buffers_.front();
			pos_ += vel_ * delta_time;
			vel_buffers_.pop_front();
		}		
		break;
	}
	case Accel: {
		// TODO: add acc mode
		if (!acc_buffers_.empty()) {
			acc_ = acc_buffers_.front();
			vel_ += acc_ * delta_time;
			pos_ += vel_ * delta_time;
			acc_buffers_.pop_front();
		}
		break;
	}
	}

	Vec3 cur_pos_	= zero_pos_ + pos_ * axis_;
	switch (type_) {
	using enum JointType;
	case Revolute: {
		ptr_parent_->setRotLocal(cur_pos_);
		break;
	}
	case Prismatic: {
		ptr_parent_->setTransLocal(cur_pos_);
		break;
	}
	}
}

std::ostream& operator<<(std::ostream& o, const JointComponent& joint_component)
{
	using std::format; using std::endl;
	o << format("Type  : JointComponent\n"
				"Parent: {: <40}\n"
				"Axis  : [{: <5}, {: < 5}, {: <5}]\n"
				"Zero  : [{: <5}, {: < 5}, {: <5}]\n"
				"Pos   : {: <6}\n",
				joint_component.ptr_parent_->getName(),
				joint_component.axis_.x(), joint_component.axis_.y(), joint_component.axis_.z(), 
				joint_component.zero_pos_.x(), joint_component.zero_pos_.y(), joint_component.zero_pos_.z(),
				joint_component.pos_);
	return o;
}

}
