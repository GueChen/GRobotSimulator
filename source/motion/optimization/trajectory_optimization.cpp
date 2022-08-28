#include "motion/optimization/trajectory_optimization.h"

#include "model/model.h"
#include "component/rigidbody_component.h"

#include "manager/physicsmanager.h"

namespace GComponent {

Twistf GComponent::TargetOptimizer::Optimize(Model & obj, const Twistf& glb_t, const std::vector<float>& thetas)
{
	PhysicsManager& physics = PhysicsManager::getInstance();
	if (!hit_infos.empty()) {
		std::cout << "The hit object list:\n";
		for (auto& [checker_index, hitter] : hit_infos) {
			//RigidBodyActor*actor = physics.GetModelIdByActorID(id);			
			//std::cout <<"hit link: " << id << " hitter: " << actor->GetParent()->getName() << std::endl;
			std::cout << "checker index: " << checker_index << std::endl;
			for (auto& [hitter_id, vec] : hitter) {
				RigidBodyActor* actor = physics.GetModelIdByActorID(hitter_id);
				std::cout << "hitter: " << actor->GetParent()->getName() << " penetration vec: " << vec.transpose() << std::endl;
			}
		}
	}
	return glb_t;
}

bool TargetOptimizer::ConditionCheck(Model& obj)
{		
	hit_infos.clear();
	return ConditionCheck(obj, PhysicsManager::getInstance().GetActivateScene().lock(), 0);
}

bool TargetOptimizer::ConditionCheck(Model& obj, const std::shared_ptr<PhysicsScene>& scene, int index)
{	
	// Self Checking
	if (RigidbodyComponent* collider_com = 
		obj.GetComponent<RigidbodyComponent>(RigidbodyComponent::type_name.data());
		collider_com) {
		std::vector<OverlapHitInfo> hit_info;	
		if (scene->Overlap(collider_com->GetActor(), 5, hit_info)) {
			hit_infos[index].insert(hit_infos[index].end(), hit_info.begin(), hit_info.end());
		}
	}

	// Children Checking
	for (auto& child : obj.getChildren()){
		ConditionCheck(*child, scene, index + 1);		
	}

	return !hit_infos.empty();
}

Eigen::Vector<float, Eigen::Dynamic> SelfmotionOptimizer::IncVector(Model&, const std::vector<float>& thetas)
{
	return Eigen::Vector<float, Eigen::Dynamic>();
}

bool SelfmotionOptimizer::ConditionCheck(Model&)
{
	return false;
}

}

