/**
 *  @file  	joint_component.h
 *  @brief 	This class is a Better abstraction for joint.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 23, 2022
 **/
#ifndef _JOINT_COMPONENT_H
#define _JOINT_COMPONENT_H

#include "component/component.hpp"

#include <eigen3/Eigen/Dense>

#include <functional>
#include <optional>
#include <mutex>
#include <list>
#include <limits>

enum class JointMode {
	Position = 0,
	Velocity = 1,
	Accel	 = 2
};

enum class JointType {
	Revolute	= 0,
	Prismatic	= 1,	
	// Hinge	= 2,
	// Shpere	= 3
};

namespace GComponent {

using Vec3		 = Eigen::Vector3f;
using Vec4		 = Eigen::Vector4f;
using Mat3		 = Eigen::Matrix3f;
using Mat4		 = Eigen::Matrix4f;
using _OptDelFun = std::optional<std::function<void(void)>>;

class JointComponent : public Component {
friend class JointGroupComponent;

public:
	struct Limitation {
		float min = std::numeric_limits<float>::lowest();
		float max = std::numeric_limits<float>::max();
	};

public:
	explicit		JointComponent(Model* ptr_parent = nullptr, Vec3 bind_axis = Vec3::Zero(), _OptDelFun del_fun = std::nullopt);
	virtual			~JointComponent() { if (opt_del_func_) opt_del_func_->operator()(); }

/// 通过 Component 继承
	virtual const string_view&
					GetTypeName() const override{ return type_name; }

/// Getter & Setter
	inline void		SetPosition(float pos)		{ pos_ = pos; }
	inline float	GetPosition()	  const		{ return pos_; }

	inline void		SetVelocity(float vel)		{ vel_ = vel;}
	inline float	GetVelocity()	  const		{ return vel_;}

	inline void		SetAcceleration(float acc)	{ acc_ = acc; }
	inline float	GetAcceleration() const		{ return acc_; }

	inline void		SetAxis(const Vec3& axis)	{axis_ = axis; }
	inline const Vec3& 
					GetAxis()		  const		{return axis_;}

/// Group Setter
	void			PushPosBuffer(float new_pos);
	inline const std::list<float>& 
					GetPosBuffer()	  const		{ return pos_buffers_;}

	void			PushVelBuffer(float new_vel);
	inline const std::list<float>&
					GetVelBuffer()    const		{ return vel_buffers_;}

	void			PushAccBuffer(float new_acc);
	inline const std::list<float>&
					GetAccBuffer()    const		{ return acc_buffers_;}

	inline void		SetPosLimit(float min_v, float max_v)
	{
		pos_limits_.min = min_v; 
		pos_limits_.max = max_v;
	}
	inline Limitation
					GetPosLimit()	  const		{ return pos_limits_; }
	bool			CheckEffective(float new_pos)	const;
	
/// Display friend method
	friend 
	std::ostream&	operator<<(std::ostream & o, const JointComponent& joint_component);

protected:
	void			tickImpl(float delta_time) override;

private:
	inline void		SetDelFunction(const _DelFun& del_fun) { opt_del_func_ = del_fun; }

protected:
	JointMode		mode_			= JointMode::Position;	// The Joint Control Mode
	JointType		type_			= JointType::Revolute;	// The Joint Type

	Vec3			axis_			= Vec3::Zero();			// The Axis Joint Bindings
	
	Vec3			zero_pos_		= Vec3::Zero();
	
	float			pos_			= 0.0f;
	float			vel_			= 0.0f;
	float			acc_			= 0.0f;
	
	std::list<float> pos_buffers_	= {};
	std::list<float> vel_buffers_	= {};
	std::list<float> acc_buffers_	= {};
	std::mutex		 pos_lock_;
	std::mutex       vel_lock_;
	std::mutex		 acc_lock_;

	_OptDelFun		 opt_del_func_  = std::nullopt;			// [Temp, Maybe D/M Later] Used for Group Component

	/* Safety item */
	Limitation		 pos_limits_;

public:
	constexpr static const std::string_view type_name = "JointComponent";
};

}


#endif // !_JOINT_COMPONENT_H
