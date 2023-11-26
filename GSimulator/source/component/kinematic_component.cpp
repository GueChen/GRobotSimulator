#include "component/kinematic_component.h"
#include "component/transform_component.h"

#include "function/conversion.hpp"

#include "base/json_serializer.hpp"

#include <algorithm>
#include <execution>

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // _DEBUG

namespace GComponent
{
using std::make_unique;

/*_____________________________________________Registered Related__________________________________________________________________________*/
static std::vector<std::function<void(const std::string&)>> kine_registered_notifiers;
static std::vector<std::function<void(const std::string&)>> kine_deregistered_notifiers;

void AddKinematicRegisterNotifier(std::function<void(const std::string&)> notifier)
{
	kine_registered_notifiers.push_back(notifier);
}
void AddKinematicDeregisterNotifier(std::function<void(const std::string&)> notifier)
{
	kine_deregistered_notifiers.push_back(notifier);
}

/*_____________________________________________Kinematic Related___________________________________________________________________________*/
KinematicComponent::KinematicComponent(Model* ptr_parent) : Component(ptr_parent)
{
	if (GetParent()) {
		UpdateExponentialCoordinates();
	}
	InitializeIKSolvers();

	for (auto& notifier : kine_registered_notifiers) {
		notifier(ptr_parent_->getName());
	}
}

KinematicComponent::KinematicComponent(const SE3<float>& initial_end_transform, Model* ptr_parent):
	Component(ptr_parent), end_transform_mat_(initial_end_transform)
{	
	if (GetParent()) {
		UpdateExponentialCoordinates();
	}
	InitializeIKSolvers();

	for (auto& notifier : kine_registered_notifiers) {
		notifier(ptr_parent_->getName());
	}
}

KinematicComponent::~KinematicComponent()
{
	for (auto& notifier : kine_deregistered_notifiers) {
		notifier(ptr_parent_->getName());
	}
}

bool KinematicComponent::ForwardKinematic(SE3<float>& out_mat)
{
	return ForwardKinematic(out_mat, GetJointsPos());
}

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
		*ik_solvers_[ik_solver_enum_],
		precision_, 
		max_iteration_, 
		decay_scaler_);
}

bool KinematicComponent::Jacobian(Jacobi<float>& out_mat, const Thetas<float>& thetas)
{
	return RobotKinematic::Jacobian(out_mat, exp_coords_, toThetav(thetas));
}

bool KinematicComponent::Jacobian(Jacobi<float>& out_mat, const Thetav<float>& thetav)
{
	return RobotKinematic::Jacobian(out_mat, exp_coords_, thetav);
}

bool KinematicComponent::ZeroProjection(ZeroProj<float>& zero_mat, const Thetas<float>& thetas)
{
	
	Jacobi<float> jaco; 
	bool result = Jacobian(jaco, thetas);
	if (result) zero_mat = jaco.transpose() * jaco;
	return result;	
}

bool KinematicComponent::ZeroProjection(ZeroProj<float>& zero_mat, const Thetav<float>& thetav)
{
	Jacobi<float> jaco;
	bool result = Jacobian(jaco, thetav);
	if (result) zero_mat = jaco.transpose() * jaco;
	return result;
}

bool KinematicComponent::Transforms(vector<SE3<float>>& out_mats, const Thetas<float>& thetas)
{
	return RobotKinematic::Transforms(out_mats, exp_coords_, toThetav(thetas));
}

bool KinematicComponent::Transforms(vector<SE3<float>>& out_mats, const Thetav<float>& thetav)
{		
	return RobotKinematic::Transforms(out_mats, exp_coords_, thetav);
}

bool KinematicComponent::DifferentialTransforms(vector<SE3<float>>& out_mats, const Thetas<float>& thetas)
{
	return RobotKinematic::Differential(out_mats, exp_coords_, toThetav(thetas));
}

bool KinematicComponent::DifferentialTransforms(vector<SE3<float>>& out_mats, const Thetav<float>& thetav)
{
	return RobotKinematic::Differential(out_mats, exp_coords_, thetav);
}

bool KinematicComponent::UpdateExponentialCoordinates()
{
	JointGroupComponent* joints_group = GetJointsGroup();
	if (joints_group) {
		joint_count_ = joints_group->GetJointsSize();
		exp_coords_.resize(joint_count_);
		/* This Function used to get parent Transform */
		auto GetTransformFnc = [](const auto& component)->SE3<float> {	
			return component->GetParent()->GetTransform()->GetModelMatrixWithoutScale();
		};

		SE3<float> inv_base_mat = InverseSE3(GetTransformFnc(this));
		
		vector<JointComponent*> joints = joints_group->GetJoints();
		SE3<float> local	 = SE3<float>::Identity();		   
		SO3<float> base_prev = SO3<float>::Identity();

		//FIXME: There is an assumption that the order of joints is in parent to children
		for (int i = 0; i < joint_count_; ++i)
		{
			SE3<float> inv_local = InverseSE3(local);
			auto [R, t] = RtDecompositionMat4((inv_base_mat * GetTransformFnc(joints[i])).eval());
			
			// Get w_b with R(t) transform
			// Get q_b with T(t) transform
			Vec3 w = base_prev * joints[i]->GetAxis();
			Vec3 q = t;

			// Transform [q(t), w(t)] to [q(0), w(0)]
			w	   = inv_local.block(0, 0, 3, 3) * w;
			q	   = AffineProduct(inv_local, q);

			// Get Twists
			exp_coords_[i]	= ScrewToTwist(q, w);

			// Get T(t) and store ^{b}_{i-1}R(t)
			local *= ExpMapping(exp_coords_[i], joints[i]->GetPosition());
			base_prev = R;
		}

		std::for_each(std::execution::par_unseq, exp_coords_.begin(), exp_coords_.end(),
			[](auto& exp_coord) {
			std::for_each(std::execution::par_unseq, exp_coord.begin(), exp_coord.end(), 
				[](auto& val) {
				if (abs(val) < 1e-6) val = 0.0f;
				});
			});		
		
		return true;
	}
	return false;
}

/*_________________________________PROTECTED METHODS_______________________________*/
void KinematicComponent::tickImpl(float delta_time)
{
	UpdateExponentialCoordinates();
}

void KinematicComponent::InitializeIKSolvers()
{
	using enum IKSolverEnum;
	ik_solvers_.emplace(LeastNorm,						make_unique<DynamicLeastNormSolver<float>>());
	ik_solvers_.emplace(DampedLeastSquare,				make_unique<DynamicDampedLeastSquareSolver<float>>(0.05f));
	ik_solvers_.emplace(JacobianTranspose,				make_unique<DynamicJacobianTransposeSolver<float>>());
	ik_solvers_.emplace(AdaptiveDampedLeastSquare,		make_unique<DynamicAdaptiveDampedLeastSquareSolver<float>>());
	ik_solvers_.emplace(SelectivelyDampedLeastSquare,	make_unique<DynamicSelectivelyDampedLeastSquareSolver<float>>());
}

Thetav<float> KinematicComponent::toThetav(const Thetas<float> thetas)
{
	return Thetav<float>::Map(thetas.data(), thetas.size());
}

Thetas<float> KinematicComponent::fromThetav(const Thetav<float> thetav)
{
	return vector<float>(thetav.begin(), thetav.end());
}

QJsonObject KinematicComponent::Save()
{
	QJsonObject com_obj;
	com_obj["type"] = type_name.data();
	com_obj["joint_count"] = (int)joint_count_;
	
	QJsonArray coords;
	for (auto& coord : exp_coords_) {
		coords.append(JSonSerializer::Serialize(coord));
	}
	com_obj["exp_coords"] = coords;
	com_obj["end_transform_mat"] = JSonSerializer::Serialize(end_transform_mat_);
	com_obj["ik_solver_enum"] = static_cast<int>(ik_solver_enum_);
	// can use enum to find solver form ik_sovlers
	com_obj["precision"]     = precision_;
	com_obj["max_iteration"] = max_iteration_;
	com_obj["decay_scaler"]  = decay_scaler_;
	
	return com_obj;
}
bool KinematicComponent::Load(const QJsonObject& com_obj)
{
	joint_count_ = com_obj["joint_count"].toInt();

	QJsonArray coords_obj = com_obj["exp_coords"].toArray();
	exp_coords_.reserve(coords_obj.size());
	for (const QJsonValue& json_val : coords_obj) {
		exp_coords_.push_back(JsonDeserializer::ToVec6f(json_val.toArray()));
	}

	QJsonArray end_mat_obj = com_obj["end_transform_mat"].toArray();
	end_transform_mat_ = JsonDeserializer::ToMat4f(end_mat_obj);

	ik_solver_enum_ = static_cast<IKSolverEnum>(com_obj["ik_solver_enum"].toInt());
	solver_ = ik_solvers_[ik_solver_enum_].get();

	precision_ = com_obj["precision"].toDouble();
	max_iteration_ = com_obj["max_iteration"].toInt();
	decay_scaler_ = com_obj["decay_scaler"].toDouble();

	return true;
}

}
