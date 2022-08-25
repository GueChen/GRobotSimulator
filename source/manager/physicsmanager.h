/**
 *  @file  	physicsmanager.h
 *  @brief 	This class posses all the physics resource and has the responsibility to create scene.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 8th, 2022
 **/
#ifndef __PHYSICS_MANAGER_H
#define __PHYSICS_MANAGER_H

#include "base/singleton.h"
#include "physics/physics_scene.h"

#include <QtCore/QObject>

#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>

/// Forward Declarations
namespace physx {
class PxFoundation;
class PxPhysics;
class PxCooking;
class PxPvd;
class PxPvdTransport;
}

//struct PxParamters;

namespace GComponent {

using std::weak_ptr;
using std::shared_ptr;
using std::unique_ptr;
using std::unordered_map;
using std::function;

class PhysicsManager : public QObject{
	friend class PhysicsScene;

	Q_OBJECT

	NonCoyable(PhysicsManager)

public:
	static PhysicsManager& getInstance();
	virtual ~PhysicsManager();
	
	void tick(float delta_time);

	void Initialize();
	void Clear();

	weak_ptr<PhysicsScene>				CreatePhysicsScene();
	void								DeletePhysicdScene(const weak_ptr<PhysicsScene>& scene_ptr);

	void								SetActivateScene(const weak_ptr<PhysicsScene>& scene_ptr);
	inline weak_ptr<PhysicsScene>		GetActivateScene() const { return activate_scenes_; }
	
	bool								RegisterActor(RigidBodyActor* model_id);
	bool								DeregisterActor(uint64_t actor_id);
	RigidBodyActor*						GetModelIdByActorID(uint64_t actor_id) const;

protected:
	PhysicsManager();

public:
//	PxParamters*						 m_params;
	
protected:
	/// table to query objects
	unordered_map<uint64_t, RigidBodyActor*>											actor_id_to_model_id_table_;	

	/// current activate physics scene
	weak_ptr<PhysicsScene>																activate_scenes_;

	/// the physics data access
	std::vector<std::shared_ptr<PhysicsScene>>												scenes_;

	/// the global resource to create pxscene
	std::unique_ptr<physx::PxFoundation,   std::function<void(physx::PxFoundation*)>>		foundations_;
	std::unique_ptr<physx::PxPhysics,      std::function<void(physx::PxPhysics*)>>			physics_;
	std::unique_ptr<physx::PxCooking,      std::function<void(physx::PxCooking*)>>			cookings_;
	std::unique_ptr<physx::PxPvd,          std::function<void(physx::PxPvd*)>>				pvd_;
	std::unique_ptr<physx::PxPvdTransport, std::function<void(physx::PxPvdTransport*)>>		transport_;

	
};

} // ~!namespace GComponent

#endif // !__PHYSICS_MANAGER_H
