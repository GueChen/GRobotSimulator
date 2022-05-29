/**
 *  @file  	joint_group_component.h
 *  @brief 	This class is used to as a Wrapper, to supply Safer and More Flexible Joint Controller.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 24, 2022
 **/

#ifndef _JOINT_GROUP_COMPONENT_H
#define _JOINT_GROUP_COMPONENT_H

#include "component/component.hpp"
#include "component/joint_component.h"

#include <optional>
#include <functional>
#include <vector>
#include <string>

namespace GComponent
{
using std::vector;
using std::string_view;
using _DelFun = std::function<void(void)>;

class JointGroupComponent:public Component {
public:
	JointGroupComponent(Model* ptr_parent, const vector<JointComponent*>& joints = {});
	~JointGroupComponent() = default;

	void tick(float delta_time)		override;

	// void Derigistered()			override;
	// 通过 Component 继承
	virtual const string_view& 
					GetTypeName()	const override	{ return type_name; }

	bool			RegisterJoint(JointComponent* joint);
	inline size_t	GetJointsSize()	const			{ return joints_.size(); }
	inline const vector<JointComponent*>& 
					GetJoints()		const			{ return joints_; }

private:
	vector<JointComponent*> joints_				 = {};

	std::optional<std::function<vector<double>()>> 
							pos_function		 = std::nullopt;

	constexpr static const string_view type_name = "JointGroupComponent";
	
};

}


#endif // !_JOINT_GROUP_COMPONENT_H
