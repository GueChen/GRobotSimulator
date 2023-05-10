#include "motion/optimization/trajectory_optimization.h"
#include "system/skinsystem.h"
#include "model/model.h"
#include "component/transform_component.h"
#include "component/rigidbody_component.h"
#include "component/kinematic_component.h"
#include "component/joint_group_component.h"
#include "manager/physicsmanager.h"

#include <GComponent/gtransform.hpp>

#include <algorithm>

#define _DEBUG
#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG


namespace GComponent {

#ifdef _DEBUG
	void DisplayJacobi(const int& checker_idx, Eigen::MatrixXf& grad_mat, GComponent::KinematicComponent& kine, const std::vector<float>& thetas, GComponent::Vec3& checker_pos, std::vector<GComponent::SE3f>& transforms)
	{
		std::cout << "checker idx: " << checker_idx << std::endl;
		std::cout << "Gradiant matrix:\n" << grad_mat.transpose() << std::endl;
		DynMat<float>jaco; kine.Jacobian(jaco, thetas);
		std::cout << "The space jacobian mat:\n" << jaco.block(3, 0, 3, 7) << std::endl;
		SE3<float> checker_trans = SE3<float>::Identity();
		checker_trans.block(0, 3, 3, 1) = checker_pos;
		jaco = Adjoint(InverseSE3(transforms[checker_idx - 1] * checker_trans)) * jaco;
		std::cout << "The obj jacobian mat:\n" << jaco.block(3, 0, 3, 7) << std::endl;
		SE3<float> local_trans = SE3<float>::Identity();
		local_trans.block(0, 0, 3, 3) = transforms[checker_idx - 1].block(0, 0, 3, 3);
		jaco = Adjoint(local_trans) * jaco;
		std::cout << "The analytic jacobian mat:\n" << jaco.block(3, 0, 3, 7) << std::endl;
	}
#endif

std::vector<float> GComponent::PhysxCheckerOptimizer::Optimize(Model & obj, const Twistf& glb_t, const std::vector<float>& thetas)
{
	std::vector<float> ret = thetas;
	if (!hit_infos_.empty()) {
		//DisplayHitterInformations(obj);
		auto& j_sdk = *obj.GetComponent<JointGroupComponent>();
		auto& kine  = *obj.GetComponent<KinematicComponent>();
		auto& trans = *obj.GetTransform();

		const int kN = j_sdk.GetJointsSize();
		std::vector<SE3f>  transforms; kine.Transforms(transforms, thetas);
		std::vector<SE3f>  difftrans;  kine.DifferentialTransforms(difftrans, thetas);
		std::partial_sum(transforms.begin(), transforms.end(), transforms.begin(), std::multiplies<>{});

		std::map<int, std::vector<SE3f>> d_trans_map;		
		std::map<int, DynMat<float>>	 grad_mat_map;
		// precaculate the grad matrix for convinience
		for (auto&& [checker_idx, actor] : hit_actors_) {
			std::vector<SE3f>& diff_cur = d_trans_map[checker_idx];
			diff_cur.resize(kN, SE3f::Zero());		
			for (int i = 0; i < checker_idx - 1; ++i) {												
				diff_cur[i] = i == 0 ? SE3f::Identity() : transforms[i - 1];
				diff_cur[i]	= diff_cur[i] * difftrans[i] * InverseSE3(transforms[i]) * transforms[checker_idx - 1];				
			}

			// one checker version
			DynMat<float>& grad_mat    = grad_mat_map[checker_idx];			
			Vec3		   checker_pos = (
								InverseSE3(transforms[checker_idx - 2]) *
								InverseSE3(trans.GetModelMatrixWithoutScale()) *
								actor.GetParent()->GetTransform()->GetModelMatrixWithoutScale() *
								actor.GetLocalModelMat()).block(0, 3, 3, 1);
			grad_mat.setZero(kN, 3);
			for (int i = 0; i < kN; ++i) {
				grad_mat.row(i) = AffineProduct(diff_cur[i], checker_pos).transpose();
			}

			DisplayJacobi(checker_idx, grad_mat, kine, thetas, checker_pos, transforms);
		}
				
		Mat3 orientation = trans.GetModelMatrixWithoutScale().block(0, 0, 3, 3);// .transpose();
		DynVec<float> ret_grad_vec = DynVec<float>::Zero(kN);		
		for (auto&& [checker_idx, hitter] : hit_infos_) {
			for (auto&& [hitter_id, vec] : hitter) {
				Vec3 local_vec = -orientation.transpose() * /*Eigen::Vector3f::UnitY() * 0.01f; */vec;
				ret_grad_vec += 0.2f * checker_idx * grad_mat_map[checker_idx] * ClampMinAbs(Eigen::VectorX<float>(local_vec), 0.005);
			}
		}
		
		std::cout << "break time stamp: " << j_sdk.GetExecutionTime() << std::endl;
		std::cout << "the modify vector:" << ret_grad_vec.transpose() << std::endl;
		std::transform(ret.begin(), ret.end(), ret_grad_vec.begin(), ret.begin(), std::plus<>{});
		if (std::find_if(hit_infos_.begin(), hit_infos_.end(), [](auto&& pair) {return pair.first >= 7; }) != hit_infos_.end()) {
			kine.InverseKinematic(ret, ExpMapping(glb_t), ret);
		}
	}
	
	return ret;
}

void PhysxCheckerOptimizer::DisplayHitterInformations(GComponent::Model& obj)
{	
	PhysicsManager& physics = PhysicsManager::getInstance();
	for (auto& [checker_index, hitter] : hit_infos_) {
		//RigidBodyActor*actor = physics.GetModelIdByActorID(id);			
		//std::cout <<"hit link: " << id << " hitter: " << actor->GetParent()->getName() << std::endl;
		std::cout << "checker index: " << checker_index << std::endl;
		for (auto& [hitter_id, vec] : hitter) {
			RigidBodyActor* actor = physics.GetModelIdByActorID(hitter_id);			
			std::cout << "hitter: " << actor->GetParent()->getName() << " penetration vec: " << vec.transpose() << std::endl;
		}
	}
}


bool PhysxCheckerOptimizer::ConditionCheck(Model& obj)
{		
	hit_infos_.clear();
	hit_actors_.clear();
	return ConditionCheck(obj, PhysicsManager::getInstance().GetActivateScene().lock(), 0);
}

bool PhysxCheckerOptimizer::ConditionCheck(Model& obj, const std::shared_ptr<PhysicsScene>& scene, int index)
{	
	// Self Checking
	if (RigidbodyComponent* collider_com = 
		obj.GetComponent<RigidbodyComponent>();
		collider_com) {
		std::vector<OverlapHitInfo> hit_info;	
		if (scene->Overlap(collider_com->GetActor(), 5, hit_info)) {
			hit_infos_[index].insert(hit_infos_[index].end(), hit_info.begin(), hit_info.end());
			hit_actors_.emplace(index, *collider_com);
		}
	}

	// Children Checking
	for (auto& child : obj.getChildren()){
		ConditionCheck(*child, scene, index + 1);		
	}

	return !hit_infos_.empty();
}


/*____________________________________Self Motion Optimizer___________________________*/
Eigen::Vector<float, Eigen::Dynamic> SelfmotionOptimizer::IncVector(Model& obj, const std::vector<float>& thetas)
{
	//return Eigen::Vector<float, Eigen::Dynamic>();
	if (!hit_infos_.empty()) {
		//DisplayHitterInformations(obj);
		auto& j_sdk = *obj.GetComponent<JointGroupComponent>();
		auto& kine  = *obj.GetComponent<KinematicComponent>();
		auto& trans = *obj.GetTransform();
		const int kN = j_sdk.GetJointsSize();
		std::vector<SE3f>  transforms; kine.Transforms(transforms, thetas);
		std::vector<SE3f>  difftrans;  kine.DifferentialTransforms(difftrans, thetas);
		std::partial_sum(transforms.begin(), transforms.end(), transforms.begin(), std::multiplies<>{});

		std::map<int, std::vector<SE3f>> d_trans_map;
		std::map<int, DynMat<float>>	 grad_mat_map;
		// precaculate the grad matrix for convinience
		for (auto&& [checker_idx, actor] : hit_actors_) {
			std::vector<SE3f>& diff_cur = d_trans_map[checker_idx];
			diff_cur.resize(kN, SE3f::Zero());
			for (int i = 0; i < checker_idx - 1; ++i) {
				diff_cur[i] = i == 0 ? SE3f::Identity() : transforms[i - 1];
				diff_cur[i] = diff_cur[i] * difftrans[i] * InverseSE3(transforms[i]) * transforms[checker_idx - 1];
			}
			// one checker version
			DynMat<float>& grad_mat = grad_mat_map[checker_idx];
			Vec3		   checker_pos = (
				InverseSE3(transforms[checker_idx - 2]) *
				InverseSE3(trans.GetModelMatrixWithoutScale()) *
				actor.GetParent()->GetTransform()->GetModelMatrixWithoutScale() *
				actor.GetLocalModelMat()).block(0, 3, 3, 1);
			/*grad_mat.setZero(kN, 3);
			for (int i = 0; i < kN; ++i) {
				grad_mat.row(i) = AffineProduct(diff_cur[i], checker_pos).transpose();
			}*/
			grad_mat.setOnes(kN, 3);

			//DisplayJacobi(checker_idx, grad_mat, kine, thetas, checker_pos, transforms);
		}

		Mat3 orientation = trans.GetModelMatrixWithoutScale().block(0, 0, 3, 3);// .transpose();
		DynVec<float> ret_grad_vec = DynVec<float>::Zero(kN);
		for (auto&& [checker_idx, hitter] : hit_infos_) {
			for (auto&& [hitter_id, vec] : hitter) {
				Vec3 local_vec = -orientation.transpose() * /*Eigen::Vector3f::UnitY() * 0.01f; */vec;
				ret_grad_vec += 0.2f * checker_idx * grad_mat_map[checker_idx] * ClampMinAbs(Eigen::VectorX<float>(local_vec), 0.005);
			}
		}
		return ret_grad_vec;
	}

}

void SelfmotionOptimizer::DisplayHitterInformations(GComponent::Model& obj)
{
	PhysicsManager& physics = PhysicsManager::getInstance();
	for (auto& [checker_index, hitter] : hit_infos_) {
		//RigidBodyActor*actor = physics.GetModelIdByActorID(id);			
		//std::cout <<"hit link: " << id << " hitter: " << actor->GetParent()->getName() << std::endl;
		std::cout << "checker index: " << checker_index << std::endl;
		for (auto& [hitter_id, vec] : hitter) {
			RigidBodyActor* actor = physics.GetModelIdByActorID(hitter_id);
			std::cout << "hitter: " << actor->GetParent()->getName() << " penetration vec: " << vec.transpose() << std::endl;
		}
	}
}

bool SelfmotionOptimizer::ConditionCheck(Model& obj)
{
	hit_infos_.clear();
	hit_actors_.clear();	
	return ConditionCheck(obj, PhysicsManager::getInstance().GetActivateScene().lock(), 0);
	//return false;
}

bool SelfmotionOptimizer::ConditionCheck(Model& obj, const std::shared_ptr<PhysicsScene>& scene, int index)
{
	// Self Checking
	if (RigidbodyComponent* collider_com =
		obj.GetComponent<RigidbodyComponent>();
		collider_com) {
		std::vector<OverlapHitInfo> hit_info;
		if (scene->Overlap(collider_com->GetActor(), 5, hit_info)) {
			hit_infos_[index].insert(hit_infos_[index].end(), hit_info.begin(), hit_info.end());
			hit_actors_.emplace(index, *collider_com);
		}
	}
	// Children Checking
	if (index < 6)
	{
		for (auto& child : obj.getChildren()) {
			ConditionCheck(*child, scene, index + 1);
		}
	}	

	return !hit_infos_.empty();
	/*float distance_ = 0;
	float safety_dis_ = 0;
	for (int i = 3; i < 5; ++i)
	{
		for (auto&& [hitter_id, vec] : hit_infos_[i]) {
			distance_ += 0.5 * pow(vec.norm(), 2);
			safety_dis_ += i * 0.005;
		}
	}

	return distance_ < safety_dis_ ? true : false;*/
}


}

