#include "joint_group_component.h"

namespace GComponent {

JointGroupComponent::JointGroupComponent(Model* ptr_parent, const vector<JointComponent*>& joints) :
	Component(ptr_parent)
{
	for (auto& joint : joints) {
		RegisterJoint(joint);
	}
}

void JointGroupComponent::tick(float delta_time)
{

}

bool JointGroupComponent::RegisterJoint(JointComponent* joint)
{
	if (!joint) return false;
	// joint not possible live longger than joints
	joint->SetDelFunction([self = joint, &joints = joints_](){	
		if(!joints.empty())
			joints.erase(
				std::remove_if(joints.begin(), joints.end(), [&self = self](auto& val) { return val == self; }), 
				joints.end());
		}
	);
	joints_.push_back(joint);	
	return true;
}

}