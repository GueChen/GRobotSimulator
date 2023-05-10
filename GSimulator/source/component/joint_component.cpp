#include "component/joint_component.h"
#include "component/transform_component.h"

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
		if (ptr_parent) {
			zero_pos_ = ptr_parent_->GetTransform()->GetRotLocal();
		}
		break;
	}
	case Prismatic: {
		if (ptr_parent) {
			zero_pos_ = ptr_parent_->GetTransform()->GetTransLocal();
		}
		break;
	}
	}
}

void JointComponent::PushPosBuffer(float new_pos)
{
	std::lock_guard<std::mutex> lock(pos_lock_);
	pos_buffers_.push_back(new_pos);
	// limit the max pos buffer size to avoid some situation
	while (pos_buffers_.size() > 10) {
		pos_buffers_.pop_front();
	}
}

void JointComponent::PushVelBuffer(float new_vel)
{
	std::lock_guard<std::mutex> lock(vel_lock_);
	vel_buffers_.push_back(new_vel);
}

void JointComponent::PushAccBuffer(float new_acc)
{
	std::lock_guard<std::mutex> lock(acc_lock_);
	acc_buffers_.push_back(new_acc); 
}

bool JointComponent::CheckEffective(float angle) const
{
	return (pos_limits_.min <= angle && angle <= pos_limits_.max)
			&&	// limitation
			abs(pos_ - angle) < (3.1415926535f / 2.0f);					// move dir limitation
}

void JointComponent::tickImpl(float delta_time)
{
	switch (mode_) {
	using enum JointMode;
	case Position: {
		std::lock_guard<std::mutex> lock(pos_lock_);
		if (!pos_buffers_.empty()) {
			pos_ = pos_buffers_.front();
			pos_buffers_.pop_front();			
		}
		else {
			return;
		}
		break;
	}
	case Velocity: {
		// TODO: add vel mode
		std::lock_guard<std::mutex> lock(vel_lock_);
		if (!vel_buffers_.empty()) {
			vel_ = pos_buffers_.front();
			pos_ += vel_ * delta_time;
			vel_buffers_.pop_front();
		}	
		else {
			return;
		}
		break;
	}
	case Accel: {
		// TODO: add acc mode
		std::lock_guard<std::mutex> lock(acc_lock_);
		if (!acc_buffers_.empty()) {

			acc_ = acc_buffers_.front();
			vel_ += acc_ * delta_time;
			pos_ += vel_ * delta_time;
			acc_buffers_.pop_front();
		}
		else {
			return;
		}
		break;
	}
	}

	Vec3 cur_pos_	= LogMapSO3Toso3( (Roderigues((pos_ * axis_).eval())* Roderigues(zero_pos_)).eval());
	switch (type_) {
	using enum JointType;
	case Revolute: {
		ptr_parent_->GetTransform()->SetRotLocal(cur_pos_);
		break;
	}
	case Prismatic: {
		ptr_parent_->GetTransform()->SetTransLocal(cur_pos_);
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
