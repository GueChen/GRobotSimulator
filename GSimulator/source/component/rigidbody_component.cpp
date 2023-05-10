#include "rigidbody_component.h"

#include "manager/physicsmanager.h"
#include "component/transform_component.h"

namespace GComponent{
	RigidbodyComponent::RigidbodyComponent(Model* ptr_parent, const Mat4& local_mat, float radius, CollisionGroup group):
		Component(ptr_parent),
		local_model_matrix_(local_mat)
	{
		SphereShape sphere_shape(radius);
		rigidbody_actor_ = CreateRigidBodyActorFromScene(sphere_shape, group);
		rigidbody_actor_->SetParent(ptr_parent_);
	}

	RigidbodyComponent::RigidbodyComponent(Model* ptr_parent, const Mat4& local_mat, float radius, float half_height, CollisionGroup group):
		Component(ptr_parent),
		local_model_matrix_(local_mat)
	{				
		CapsuleShape capsule_shape(radius, half_height);
		rigidbody_actor_ = CreateRigidBodyActorFromScene(capsule_shape, group);
		rigidbody_actor_->SetParent(ptr_parent_);
	}

	RigidbodyComponent::RigidbodyComponent(Model* ptr_parent, const Mat4& local_mat, float half_x, float half_y, float half_z, CollisionGroup group):
		Component(ptr_parent),
		local_model_matrix_(local_mat)
	{
		BoxShape box_shape(half_x, half_y, half_z);
		rigidbody_actor_ = CreateRigidBodyActorFromScene(box_shape, group);
		rigidbody_actor_->SetParent(ptr_parent_);
	}

	RigidbodyComponent::~RigidbodyComponent()
	{
		std::shared_ptr<PhysicsScene> activate_scene = PhysicsManager::getInstance().GetActivateScene().lock();
		activate_scene->RemoveRigidBodyActor(rigidbody_actor_);
		delete rigidbody_actor_;
	}

	void RigidbodyComponent::tickImpl(float delta_time)
	{
		TransformCom& trans = *ptr_parent_->GetTransform();
		rigidbody_actor_->SetGlobalTransform(trans.GetModelMatrixWithoutScale());
	}

	RigidBodyActor* RigidbodyComponent::CreateRigidBodyActorFromScene(AbstractShape& shape, CollisionGroup group)
	{
		std::shared_ptr<PhysicsScene> activate_scene = PhysicsManager::getInstance().GetActivateScene().lock();		
		TransformCom& trans = *ptr_parent_->GetTransform();
		return activate_scene->CreateRigidBodyActor(trans.GetModelMatrixWithoutScale(), local_model_matrix_, shape, group);
	}

}// !namespace GComponent
