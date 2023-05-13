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
using Limit = JointComponent::Limitation;

public:
	explicit JointGroupComponent(Model* ptr_parent = nullptr, const vector<JointComponent*>& joints = {});
	~JointGroupComponent();

	bool			RegisterJoint(JointComponent* joint);
	// void Derigistered()			override;

	virtual const string_view& 
					GetTypeName		()	const override	{ return type_name; }	
	inline size_t	GetJointsSize	()	const			{ return joints_.size(); }
	inline	const vector<JointComponent*>& 
					GetJoints		()	const			{ return joints_; }

	vector<float>	GetPositions	()  const;
	void			SetPositions	(const std::vector<float>&positions);
	void			SetPositionsWithTimeStamp
									(const std::vector<float>&positions, float time_stamp);
	inline	float   GetExecutionTime()  const			{ return execution_time_; }

	bool			SafetyCheck		(const std::vector<float>& positions) const;
	void			SetLimitations	(const std::vector<float>& min_lims,
								     const std::vector<float>& max_lims);
	std::vector<Limit>
					GetLimitations()									  const;

	int				SearchJointsInChildren();
	
protected:
	void tickImpl(float delta_time)				 override;
	QJsonObject Save()							 override;
	bool	    Load(const QJsonObject& com_obj) override;
	bool		LazyLoad()						 override;

private:
	vector<JointComponent*>			joints_				   = {};
	// TODO: consider the situation under tow paralle joints
	vector<JointComponent*>			joinable_joints_	   = {};
	unordered_set<JointComponent*>	record_table_		   = {};
	
	float							execution_time_		   = -1.0f;
	std::list<float>				execution_time_buffer_ = {};

public:
	constexpr static const string_view type_name = "JointGroupComponent";
	
};

}


#endif // !_JOINT_GROUP_COMPONENT_H
