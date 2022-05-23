#include "joint_component.h"

void GComponent::JointComponent::tick(float delta_time)
{
	switch (cur_mode_) {
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

}
