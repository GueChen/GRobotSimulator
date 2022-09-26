#include "manager/physicsmanager.h"

#include <PxPhysicsAPI.h>

namespace GComponent {

PhysicsManager::PhysicsManager()  = default;
PhysicsManager::~PhysicsManager() {
	transport_->disconnect();
}

void PhysicsManager::tick(float delta_time)
{
	for (auto& scene : scenes_) {
		scene->tick(delta_time);
	}
}


PhysicsManager& PhysicsManager::getInstance()
{
	static PhysicsManager instance;
	return instance;
}

void PhysicsManager::Initialize()
{
	static physx::PxDefaultAllocator		default_allocator_callback;
	static physx::PxDefaultErrorCallback	default_error_callback;
	static physx::PxTolerancesScale			default_scale;

	constexpr char PvdHost[] = "127.0.0.1";

	auto physx_deleter = [](auto ptr) { ptr->release(); };
	foundations_	= std::unique_ptr<physx::PxFoundation, std::function<void(physx::PxFoundation*)>>(
						PxCreateFoundation(PX_PHYSICS_VERSION, default_allocator_callback, default_error_callback), 
						physx_deleter);
	pvd_			= std::unique_ptr<physx::PxPvd, std::function<void(physx::PxPvd*)>>(
						physx::PxCreatePvd(*foundations_),
						physx_deleter);
	transport_		= std::unique_ptr<physx::PxPvdTransport, std::function<void(physx::PxPvdTransport*)>>(
						physx::PxDefaultPvdSocketTransportCreate(PvdHost, 5425, 10),
						physx_deleter);
	cookings_		= std::unique_ptr<physx::PxCooking, std::function<void(physx::PxCooking*)>>(
						PxCreateCooking(PX_PHYSICS_VERSION, *foundations_, physx::PxCookingParams(default_scale)),
						physx_deleter);
	physics_		= std::unique_ptr<physx::PxPhysics, std::function<void(physx::PxPhysics*)>>(
						PxCreatePhysics(PX_PHYSICS_VERSION, *foundations_, default_scale, true, pvd_.get()),
						[](physx::PxPhysics* ptr){
							ptr->release();
						});
	PxInitExtensions(*physics_, pvd_.get());
	pvd_->connect(*transport_, physx::PxPvdInstrumentationFlag::eALL);

}

void PhysicsManager::Clear()
{
	scenes_.clear();
}

std::weak_ptr<PhysicsScene> PhysicsManager::CreatePhysicsScene()
{
	std::shared_ptr<PhysicsScene> physics_scene = std::make_shared<PhysicsScene>();
	
	scenes_.push_back(physics_scene);
	
	return physics_scene;
}

void PhysicsManager::DeletePhysicdScene(const std::weak_ptr<PhysicsScene>& scene_ptr)
{
	std::shared_ptr<PhysicsScene> ptr = scene_ptr.lock();
	auto iters = std::ranges::find(scenes_, ptr);
	if (iters != scenes_.end()) {
		scenes_.erase(iters);
	}	
}

void PhysicsManager::SetActivateScene(const std::weak_ptr<PhysicsScene>& scene_ptr)
{
	activate_scenes_ = scene_ptr;
}

bool PhysicsManager::RegisterActor(RigidBodyActor* actor)
{
	if (!actor) return false;
	actor_id_to_model_id_table_.emplace(actor->GetActorID(), actor);
	return true;
}

bool PhysicsManager::DeregisterActor(uint64_t actor_id)
{
	auto iter = actor_id_to_model_id_table_.find(actor_id);
	if (iter == actor_id_to_model_id_table_.end()) return false;
	actor_id_to_model_id_table_.erase(actor_id);
	return true;
}

RigidBodyActor* PhysicsManager::GetModelIdByActorID(uint64_t actor_id) const
{
	auto iter = actor_id_to_model_id_table_.find(actor_id);
	if (iter == actor_id_to_model_id_table_.end()) return 0;
	return iter->second;
}

}