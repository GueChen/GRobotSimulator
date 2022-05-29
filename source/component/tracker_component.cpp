#include "component/tracker_component.h"

// TODO: Replace it with Kinematic Component
#include "function/conversion.hpp"

#include "model/robot/kuka_iiwa_model.h"
#include "model/model.h"

void GComponent::TrackerComponent::tick(float delta_time)
{
	Model* ptr_parent = GetParent();
	if (!tracked_goal_ || !ptr_parent) return;
	//Kinematic* robot_kinematic_component = ptr_parent->GetComponents<Kinematic>("Kinematic");
	KUKA_IIWA_MODEL* robot_component = static_cast<KUKA_IIWA_MODEL*>(ptr_parent);
	mat4 goal_global_SE3 = tracked_goal_->getModelMatrixWithoutScale();
	mat4 goal_local_SE3 = inverse(robot_component->getModelMatrix()) * goal_global_SE3;
	robot_component->Move(robot_component->BackKinematic(Conversion::toMat4d(goal_local_SE3)));
}
