#include "component/kinematic_component.h"

#include "function/conversion.hpp"

#include <algorithm>
#include <execution>

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG

namespace GComponent
{
KinematicComponent::KinematicComponent(Model* ptr_parent): Component(ptr_parent)
{}

bool KinematicComponent::ForwardKinematic(SE3<float>&out_mat, const Thetas<float>&thetas)
{
	return ForwardKinematic(out_mat, toThetav(thetas));
}
bool KinematicComponent::ForwardKinematic(SE3<float>& out_mat, const Thetav<float>& thetav)
{
	return RobotKinematic::ForwardKinematic(out_mat, end_transform_mat_, exp_coords_, thetav);
}

bool KinematicComponent::InverseKinematic(Thetas<float>& out_thetas, const SE3<float>& trans_desire, const Thetas<float>& init_guess)
{
	Thetav<float> out_thetav;
	bool result = InverseKinematic(out_thetav, trans_desire, toThetav(init_guess));
	if (result) {
		out_thetas = fromThetav(out_thetav);
	}
	return result;
}
bool KinematicComponent::InverseKinematic(Thetav<float>& out_thetav, const SE3<float>& trans_desire, const Thetav<float>& init_guess)
{
	return 	RobotKinematic::InverseKinematic(
		out_thetav, 
		end_transform_mat_, 
		exp_coords_, 
		trans_desire, 
		init_guess, 
		*solver_, 
		precision_, 
		max_iteration_, 
		decay_scaler_);;
}

bool KinematicComponent::UpdateExponentialCoordinates()
{
	JointGroupComponent* joints_group = GetJointsGroup();
	if (joints_group) {
		joint_count_ = joints_group->GetJointsSize();
		exp_coords_.resize(joint_count_);
		/* This Function used to get parent Transform */
		auto GetTransformFnc = [](const auto& component)->SE3<float> {
			//return Conversion::toMat4d(component->GetParent()->getModelMatrixWithoutScale());
			return component->GetParent()->getModelMatrixWithoutScale();
		};

		SE3<float> inv_base_mat = InverseSE3(GetTransformFnc(this));

		/* This Fnc Get the Joint Msg pair [pos, axis] */
		auto GetJointScrewFnc = [&GetTFnc = GetTransformFnc, &inv_base_mat](const auto& component)->std::pair<Vec3, Vec3> {
			return std::make_pair(
				(inv_base_mat * GetTFnc(component)).block(0, 3, 3, 1),
				//Conversion::toVec3d(component->GetAxis()));
				component->GetAxis());
		};

		vector<JointComponent*> joints = joints_group->GetJoints();
		SE3<float> local = SE3<float>::Identity();

		//FIXME: There is an assumption that the order of joints is in parent to children
		for (int i = 0; i < joint_count_; ++i)
		{
			auto [q, w]		= GetJointScrewFnc(joints[i]);
			q				= affineProduct(InverseSE3(local), q);
			exp_coords_[i]	= ScrewToTwist(q, w);
			if (i < joint_count_ - 1) {
				local *= ExpMapping(exp_coords_[i], float(joints[i]->GetPosition()));
			}
		}
		end_transform_mat_ = local;

		for (auto& exp_coord : exp_coords_) {
			for (auto& val : exp_coord) {
				if (abs(val) < 1e-6) val = 0.0;
			}
		}
		return true;
	}
	return false;
}

void KinematicComponent::tickImpl(float delta_time)
{
		
}

Thetav<float> KinematicComponent::toThetav(const Thetas<float> thetas)
{
	return Thetav<float>::Map(thetas.data(), thetas.size());
}

Thetas<float> KinematicComponent::fromThetav(const Thetav<float> thetav)
{
	return vector<float>(thetav.begin(), thetav.end());
}


}
