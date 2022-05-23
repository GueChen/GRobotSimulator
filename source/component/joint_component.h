/**
 *  @file  	joint_component.h
 *  @brief 	This class is a Better abstraction for joint.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 23, 2022
 **/
#ifndef _JOINT_COMPONENT_H
#define _JOINT_COMPONENT_H

#include "component/component.hpp"

#include <list>

enum class JointMode {
	Position = 0,
	Velocity = 1,
	Accel	 = 2
};

namespace GComponent {
class JointComponent : public Component {
public:
	explicit		JointComponent(Model* ptr_parent) : Component(ptr_parent) {}
	virtual			~JointComponent() {}
	
	inline void		SetPosition(float pos)		{ pos_ = pos; }
	inline float	GetPosition()	  const		{ return pos_; }

	inline void		SetVelocity(float vel)		{ vel_ = vel;}
	inline float	GetVelocity()	  const		{ return vel_;}

	inline void		SetAcceleration(float acc)	{ acc_ = acc; }
	inline float	GetAcceleration() const		{ return acc_; }

	inline void		PushPosBuffer(float new_pos){ pos_buffers_.push_back(new_pos); }
	inline const std::list<float>& 
					GetPosBuffer() const		{ return pos_buffers_;}

	inline void		PushVelBuffer(float new_vel){ vel_buffers_.push_back(new_vel); }
	inline const std::list<float>&
					GetVelBuffer() const		{ return vel_buffers_;}

	inline void		PushAccBuffer(float new_acc){ acc_buffers_.push_back(new_acc); }
	inline const std::list<float>&
					GetAccBuffer() const		{ return acc_buffers_;}

	void			tick(float delta_time) override;

protected:
	JointMode		cur_mode_		= JointMode::Position;

	float			pos_			= 0.0f;
	float			vel_			= 0.0f;
	float			acc_			= 0.0f;

	std::list<float> pos_buffers_;
	std::list<float> vel_buffers_;
	std::list<float> acc_buffers_;

};

}


#endif // !_JOINT_COMPONENT_H
