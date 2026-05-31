#ifndef ROBOT_MODEL_H
#define ROBOT_MODEL_H

#include "component/joint_component.h"
#include "component/joint_group_component.h"
#include "component/kinematic_component.h"
#include "model/model.h"

#include <utility>
#include <vector>

namespace GComponent {

class RobotModel : public Model {
public:
	explicit RobotModel(Model* parent = nullptr, const std::string& mesh_key = "")
		: Model(parent, mesh_key)
	{
	}

	const std::vector<Model*>& GetLinks() const { return links_; }

	Model* GetLink(size_t index) const
	{
		return index < links_.size() ? links_[index] : nullptr;
	}

	JointGroupComponent* GetJointGroup() const
	{
		for (const auto& component : GetComponents()) {
			if (component->GetTypeName() == JointGroupComponent::type_name) {
				return dynamic_cast<JointGroupComponent*>(component.get());
			}
		}
		return nullptr;
	}

	KinematicComponent* GetKinematic() const
	{
		for (const auto& component : GetComponents()) {
			if (component->GetTypeName() == KinematicComponent::type_name) {
				return dynamic_cast<KinematicComponent*>(component.get());
			}
		}
		return nullptr;
	}

	std::vector<float> GetJointPositions() const
	{
		if (auto* joint_group = GetJointGroup()) {
			return joint_group->GetPositions();
		}
		return {};
	}

	void SetJointPositions(const std::vector<float>& positions)
	{
		if (auto* joint_group = GetJointGroup()) {
			joint_group->SetPositions(positions);
		}
	}

	std::vector<JointComponent::Limitation> GetJointLimits() const
	{
		if (auto* joint_group = GetJointGroup()) {
			return joint_group->GetLimitations();
		}
		return {};
	}

protected:
	void SetRobotRuntimeData(std::vector<Model*> links, std::vector<JointComponent*> joints)
	{
		links_ = std::move(links);
		joints_ = std::move(joints);
	}

	const std::vector<JointComponent*>& GetJointComponents() const { return joints_; }

private:
	std::vector<Model*> links_;
	std::vector<JointComponent*> joints_;
};

} // namespace GComponent

#endif // ROBOT_MODEL_H
