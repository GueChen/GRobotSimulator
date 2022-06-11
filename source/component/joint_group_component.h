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

#include <functional>
#include <optional>
#include <unordered_set>
#include <vector>
#include <string>
#include <list>

namespace GComponent
{
using std::list;
using std::vector;
using std::string_view;
using std::unordered_set;
using _DelFun = std::function<void(void)>;

class JointGroupComponent:public Component {
public:
	explicit JointGroupComponent(Model* ptr_parent, const vector<JointComponent*>& joints = {});
	~JointGroupComponent();

	bool			RegisterJoint(JointComponent* joint);
	// void Derigistered()			override;

	virtual const string_view& 
					GetTypeName()	const override	{ return type_name; }	
	inline size_t	GetJointsSize()	const			{ return joints_.size(); }
	inline const vector<JointComponent*>& 
					GetJoints()		const			{ return joints_; }

	vector<float>	GetPositions();
	void			SetPositions(const vector<float>& positions);

	int				SearchJointsInChildren();
	
protected:
	void tickImpl(float delta_time)		override		   {}

private:
	vector<JointComponent*>			joints_				 = {};
	// TODO: consider the situation under tow paralle joints
	vector<JointComponent*>			joinable_joints_	 = {};
	unordered_set<JointComponent*>	record_table_		 = {};

	std::optional<std::function<vector<double>()>> 
							pos_function		 = std::nullopt;

	constexpr static const string_view type_name = "JointGroupComponent";
	
};

}


#endif // !_JOINT_GROUP_COMPONENT_H
