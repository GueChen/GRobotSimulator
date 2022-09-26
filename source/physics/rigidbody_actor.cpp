#include "physics/rigidbody_actor.h"

#include "manager/physicsmanager.h"

namespace GComponent{

void GComponent::RigidBodyActor::SetGlobalTransform(const Mat4& transform_mat)
{
	global_transform_ = transform_mat;
	std::shared_ptr<PhysicsScene> scene = PhysicsManager::getInstance().GetActivateScene().lock();
	scene->UpdateRigidBodyActorTransfrom(this, transform_mat);

}

void RigidBodyActor::AttachShape(AbstractShape& shape_interface)
{
	switch (shape_interface.GetShapeType()) {
		using enum ShapeEnum;
	case Sphere: {
		SphereShape& concret_shape = dynamic_cast<SphereShape&>(shape_interface);
		actor_shapes_.push_back(std::make_shared<SphereShape>(concret_shape));
		break;
	}
	case Capsule: {
		CapsuleShape& concret_shape = dynamic_cast<CapsuleShape&>(shape_interface);
		actor_shapes_.push_back(std::make_shared<CapsuleShape>(concret_shape));
		break;
	}
	case Box: {
		BoxShape& concret_shape = dynamic_cast<BoxShape&>(shape_interface);
		actor_shapes_.push_back(std::make_shared<BoxShape>(concret_shape));
		break;
	}
	}
}

}