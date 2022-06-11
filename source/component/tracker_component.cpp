#include "component/tracker_component.h"

// TODO: Replace it with Kinematic Component
#include "model/model.h"
#include "component/kinematic_component.h"
#include "component/joint_group_component.h"

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG


void GComponent::TrackerComponent::tickImpl(float delta_time)
{
	static int count = 0;
	Model* ptr_parent = GetParent();
	if (!tracked_goal_ || !ptr_parent) return;
	Mat4 goal_local_SE3  = InverseSE3(ptr_parent->getModelMatrix()) * tracked_goal_->getModelMatrixWithoutScale();

	KinematicComponent*  kinematic_component   = ptr_parent->GetComponet<KinematicComponent>("KinematicComponent");
	
	Thetas<float> out_thetas;
	if (kinematic_component) {
		JointGroupComponent* joint_group_component = ptr_parent->GetComponet<JointGroupComponent>("JointGroupComponent");		 
		
		bool result  = kinematic_component->InverseKinematic(out_thetas, goal_local_SE3, joint_group_component->GetPositions());
		std::ranges::transform(out_thetas, out_thetas.begin(), ToStandarAngle);
		if (result) {
			joint_group_component->SetPositions(out_thetas);
			Mat4 cur_trans;
			kinematic_component->ForwardKinematic(cur_trans, out_thetas);
			std::cout << std::format("residual: {: <12}\n", LogMapSE3Tose3((InverseSE3(cur_trans) * goal_local_SE3).eval()).norm());
		}
		else {
			std::cout << std::format("failure {: <5}\n", ++count);
		}
		
	}
	
}
