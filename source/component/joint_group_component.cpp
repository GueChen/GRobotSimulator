#include "joint_group_component.h"

#include "model/model.h"

#include <stack>
#include <algorithm>
#include <execution>
#include <ranges>

namespace GComponent {

using std::stack;

JointGroupComponent::JointGroupComponent(Model* ptr_parent, const vector<JointComponent*>& joints) :
	Component(ptr_parent)
{
	for (auto& joint : joints) {
		RegisterJoint(joint);
	}
}

JointGroupComponent::~JointGroupComponent()
{
	for (auto& joint : joints_) {
		joint->opt_del_func_ = std::nullopt;
	}
}

vector<float> JointGroupComponent::GetPositions() const
{
	vector<float> positions(GetJointsSize());
	std::transform(std::execution::par_unseq,
		joints_.begin(), joints_.end(), positions.begin(),
		[](const auto& joint) {
			return joint->GetPosition();
		});
	return positions;
}

void JointGroupComponent::SetPositions(const vector<float>& positions)
{
	for (int i = 0; i < joints_.size(); ++i) {
		joints_[i]->PushPosBuffer(positions[i]);
	}
}

bool JointGroupComponent::SafetyCheck(const std::vector<float>& positions) const
{
	assert(positions.size() <= joints_.size() && 
		   "JointGroupComponent::SafetyCheck::checking joints count greater than configure count\n");
	for (int i = 0; i < positions.size(); ++i) {
		if (!joints_[i]->CheckEffective(positions[i])) {
			return false;
		}
	}
	return true;
}

void JointGroupComponent::SetLimitations(const std::vector<float>& min_lims, const std::vector<float>& max_lims)
{
	assert(min_lims.size() == joints_.size() && max_lims.size() == joints_.size() &&
		   "JointGroupComponent::SetLimitations::the limits count not match to joints size\n");
	for (int i = 0; i < joints_.size(); ++i) {
		JointComponent& joint = *joints_[i];
		joint.SetPosLimit(min_lims[i], max_lims[i]);
	}
}

std::vector<JointGroupComponent::Limit> JointGroupComponent::GetLimitations() const
{
	std::vector<Limit> ret(joints_.size());
	for (int i = 0; i < joints_.size(); ++i) {
		ret[i] = joints_[i]->GetPosLimit();
	}
	return ret;
}

int JointGroupComponent::SearchJointsInChildren()
{
	Model* parent = GetParent();
	if (!parent) return 0;
	stack<Model*> search_stack;
	search_stack.push(parent);
	
	while (!search_stack.empty())
	{
		Model* cur = search_stack.top();
		search_stack.pop();
		for (auto& child : cur->getChildren()) 
		{
			JointComponent* joint = child->GetComponent<JointComponent>("JointComponent");
			if (joint && !record_table_.count(joint))
			{
				joinable_joints_.push_back(joint);
				record_table_.insert(joint);
				joint->SetDelFunction([self = joint, &joints = joinable_joints_, &record = record_table_]() {
					joints.erase(
						std::remove_if(joints.begin(), joints.end(), [&self = self](auto& val) { return val == self; }),
						joints.end());
					record.erase(self);
					}
				);
				search_stack.push(child);
			}
		}
	}

	return joints_.size();
}

bool JointGroupComponent::RegisterJoint(JointComponent* joint)
{
	if (!joint) return false;
	// joint not possible live longger than joints
	joint->SetDelFunction([self = joint, &joints = joints_, &record = record_table_]() {		
			joints.erase(
				std::remove_if(joints.begin(), joints.end(), [&self = self](auto& val) { return val == self; }),
				joints.end());
			record.erase(self);
		}
	);
	joints_.push_back(joint);	
	record_table_.insert(joint);
	return true;
}

}