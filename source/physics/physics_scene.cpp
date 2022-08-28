#include "physics/physics_scene.h"

#include "physics/physx/physx_utils.h"
#include "manager/physicsmanager.h"

#include <PxPhysicsAPI.h>

#include <ranges>

#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG

using namespace physx;

namespace GComponent {

PxFilterFlags CustomFilterShader(
	PxFilterObjectAttributes attributes0, PxFilterData filterData0,
	PxFilterObjectAttributes attributes1, PxFilterData filterData1, 
	PxPairFlags& pair_flags, const void* constantBlock, PxU32 constantBlockSize) 
{
	if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1)) {
		pair_flags = PxPairFlag::eTRIGGER_DEFAULT;
		return PxFilterFlag::eDEFAULT;
	}

	pair_flags = PxPairFlag::eCONTACT_DEFAULT;

	if (filterData0.word0 == filterData1.word0) {
		pair_flags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
		return PxFilterFlag::eSUPPRESS;
	}
	
	return PxFilterFlag::eDEFAULT;
}

class SimpleQueryCallBack : public PxSceneQueryFilterCallback {
public:
	SimpleQueryCallBack() = default;
	// Í¨¹ý PxQueryFilterCallback ¼Ì³Ð
	virtual PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags) override
	{				
		if (shape->getQueryFilterData() == filterData) {
			return PxQueryHitType::eNONE;
		}
		else {
			return PxQueryHitType::eTOUCH;
		}		
	}
	virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit) override
	{
		return PxQueryHitType::Enum();
	}

};

/// <summary>
/// this class is used to extend all the api could use
/// </summary>
struct PhysicsImpl
{
	PxScene*		 m_scene		  = nullptr;
	PxCpuDispatcher* m_cpu_dispatcher = nullptr;	
};

PhysicsScene::PhysicsScene():
	physics_impl_(std::make_shared<PhysicsImpl>())
{
	PxPhysics&		  physics_sdk    = *PhysicsManager::getInstance().physics_;
	PxSceneDesc		  descriptor(physics_sdk.getTolerancesScale());	
	PxCpuDispatcher*  cpu_dispatcher = PxDefaultCpuDispatcherCreate(4);
	descriptor.gravity				 = PxVec3(0.0f);
	descriptor.cpuDispatcher		 = cpu_dispatcher;
	descriptor.filterShader			 = CustomFilterShader;
											  /*PxDefaultSimulationFilterShader;	*/
	physics_impl_->m_cpu_dispatcher	 = cpu_dispatcher;	
	physics_impl_->m_scene			 = physics_sdk.createScene(descriptor);
	
}

PhysicsScene::~PhysicsScene()
{	
	physics_impl_->m_scene->release();
}

void PhysicsScene::tick(float time_step)
{
	std::lock_guard<std::mutex> lock(mutex_lock_);
	physics_impl_->m_scene->simulate(time_step);
	physics_impl_->m_scene->fetchResults(true);
}

RigidBodyActor* PhysicsScene::CreateRigidBodyActor(const Mat4& global_mat, const Mat4& local_mat, AbstractShape& shape_res, CollisionGroup group)
{
	std::lock_guard<std::mutex> lock(mutex_lock_);
	// the meterial paramters in the scene about shapes
	const float			kDensity			= 1.0f, 
						kStaticFriction		= 0.5f,
						kDynamicFriction	= 0.45f,
						kRestitutions		= 0.1f;
	// the glb resource from physics manager
	PhysicsManager&		glb_resource		= PhysicsManager::getInstance();
	PxPhysics&			physx_sdk			= *glb_resource.physics_;

	// cast to the right shape
	//TODO: using a better pattern to process this part
	std::shared_ptr<PxGeometry>	px_geometry	= PhysXUtils::fromShape(shape_res);

	// create an actor in physx scene
	PxMaterial&			default_material	= *physx_sdk.createMaterial(kStaticFriction, kDynamicFriction, kRestitutions);		
	PxShape&			px_capsule_shape	= *physx_sdk.createShape(*px_geometry, default_material, true);
	px_capsule_shape.setQueryFilterData(PxFilterData(group[0], group[1], group[2], group[3]));
	px_capsule_shape.setLocalPose(PxTransform(PhysXUtils::fromMat4f(local_mat)));
	PxRigidDynamic*		px_dyn_actor		= PxCreateDynamic(physx_sdk, PxTransform(PhysXUtils::fromMat4f(global_mat)), 
															  px_capsule_shape, 
															  kDensity);		
	
	physics_impl_->m_scene->addActor(*px_dyn_actor);

	// fill information into the rigidbody actor
	RigidBodyActor*		rigid_body_actor	= new RigidBodyActor(px_dyn_actor->getInternalIslandNodeIndex());
	glb_resource.RegisterActor(rigid_body_actor);
	rigid_body_actor->AttachShape(shape_res);

	return rigid_body_actor;
}

void PhysicsScene::RemoveRigidBodyActor(RigidBodyActor* actor)
{
	std::lock_guard<std::mutex> lock(mutex_lock_);
	PxActorTypeFlags	actor_type_flag = PxActorTypeFlag::eRIGID_DYNAMIC;
	PxScene&			scene			= *physics_impl_->m_scene;
	// get all actor from scene
	vector<PxActor*>	actors(scene.getNbActors(actor_type_flag));
	scene.getActors(actor_type_flag, actors.data(), actors.size());
	// erase the actor
	std::erase_if(actors, [actor_id = actor->actor_id_](PxActor* actor) {
		return  dynamic_cast<PxRigidBody*>(actor)->getInternalIslandNodeIndex() == actor_id;
	});	

	PhysicsManager::getInstance().DeregisterActor(actor->actor_id_);
}

void PhysicsScene::UpdateRigidBodyActorTransfrom(RigidBodyActor* actor, const Mat4& mat)
{
	std::lock_guard<std::mutex> lock(mutex_lock_);
	PxActorTypeFlags	actor_type_flag = PxActorTypeFlag::eRIGID_DYNAMIC;
	PxScene&			scene			= *physics_impl_->m_scene;
	// get all actor from scene
	vector<PxActor*>	actors(scene.getNbActors(actor_type_flag));
	
	scene.getActors(actor_type_flag, actors.data(), actors.size());
	std::ranges::for_each(actors, [actor_id = actor->actor_id_, &mat](PxActor* actor){
		PxRigidBody* rigidbody_actor = dynamic_cast<PxRigidBody*>(actor);
		if (rigidbody_actor && rigidbody_actor->getInternalIslandNodeIndex() == actor_id) {
			rigidbody_actor->setGlobalPose(PxTransform(PhysXUtils::fromMat4f(mat)));
		}
	});
}

bool PhysicsScene::Overlap(RigidBodyActor& actor, uint32_t max_hits, vector<OverlapHitInfo>& out_hits)
{
	std::lock_guard<std::mutex> lock(mutex_lock_);
	PhysicsManager&	glb_manager = PhysicsManager::getInstance();
	PxScene&		scene		= *physics_impl_->m_scene;
	bool			hit_any		= false;
	PxTransform		checker_loc = PxTransform(PhysXUtils::fromMat4f(actor.global_transform_));
	out_hits.clear();

	SimpleQueryCallBack	query_callback;
	PxQueryFilterData   query_config;
	query_config.data  = { actor.group_msg_[0], actor.group_msg_[1], actor.group_msg_[2], actor.group_msg_[3] };
	query_config.flags |= PxQueryFlag::ePREFILTER;
	
	for (auto & shape : actor.actor_shapes_ ) {
		shared_ptr<PxGeometry>	checker_geom = PhysXUtils::fromShape(*shape);
		vector<PxOverlapHit>	hit_info(max_hits);		
		uint32_t				num			= PxSceneQueryExt::overlapMultiple(
															   scene, 
															   *checker_geom,   checker_loc,
															   hit_info.data(), max_hits, 
															   query_config,    &query_callback);
		hit_info.resize(num);
		if (!hit_info.empty()){
			hit_any = true;
			for (auto& hit : hit_info) {
				uint64_t		hitter_id   = dynamic_cast<PxRigidDynamic*>(hit.actor)->getInternalIslandNodeIndex();
				RigidBodyActor*	hit_actor	= glb_manager.GetModelIdByActorID(hitter_id);						
				PxVec3			dir			= PxVec3(0.0f);
				float			depth		= 0.0f;
				PxGeometry&		hitter_geom = hit.shape->getGeometry().any();
								
				if (PxGeometryQuery::computePenetration(dir, depth, *checker_geom, checker_loc, hitter_geom, hit.actor->getGlobalPose())) {				
					out_hits.emplace_back(hitter_id, PhysXUtils::toVec3f(depth * dir));
				}
			}
		}
	}
	return hit_any;
}

bool PhysicsScene::Sweep(RigidBodyActor& actor, const Vec3& dir, float dist, vector<SweepHitInfo>& hits)
{
	std::lock_guard<std::mutex> lock(mutex_lock_);
	PxScene& scene	= *physics_impl_->m_scene;
	bool			hit_any = false;
	for (auto & shape : actor.actor_shapes_ ) {
		PxSweepBuffer hit_info;		
		bool result =  scene.sweep(*PhysXUtils::fromShape(*shape),
								   PxTransform(PhysXUtils::fromMat4f(actor.global_transform_)),
								   PhysXUtils::fromVec3f(dir.normalized()),
								   dist,
								   hit_info);
		hit_any		|= result;
		if (result){
#ifdef _DEBUG
			std::cout << "Sweep there\n";
#endif
		}
	}
	return hit_any;
	return false;
}

bool PhysicsScene::Raycast(Vec3 ori, Vec3 dir, float max_dist, vector<RaycastHitInfo>& hits)
{
	std::lock_guard<std::mutex> lock(mutex_lock_);
	// TODO: add method to this class for mutex lock checking
	return false;
}




} // !namespace GComponent