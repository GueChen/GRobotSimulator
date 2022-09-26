/**
 *  @file  	rigidbody_actor.h
 *  @brief 	This file is a wrapper for physics scene.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 9th, 2022
 **/
#ifndef __RIGIDBODY_ACTOR_H
#define __RIGIDBODY_ACTOR_H

#include "model/model.h"
#include "physics/abstract_shape.hpp"

#include <vector>
#include <memory>

namespace GComponent {

using std::vector;
using std::shared_ptr;

struct CollisionGroup {
	uint32_t words[4] = { 0 };

	CollisionGroup() = default;
	CollisionGroup(uint32_t word_1, uint32_t word_2, uint32_t word_3, uint32_t word_4) {
		words[0] = word_1; words[1] = word_2; words[2] = word_3;words[3] = word_4;
	}
	
/// Methods for convinients:
	inline uint32_t& operator[](size_t pos) { return words[pos]; }
	
};

// TODO: use weak_ptr and shared_ptr replace raw_ptr with unique_ptr
class RigidBodyActor {
/// Friends
	friend class PhysicsScene;

/// Type Alias
	using _ShapePtrs = vector<shared_ptr<AbstractShape>>;

/// Methods
public:
	RigidBodyActor(uint32_t id) :actor_id_(id) {}
	~RigidBodyActor() = default;
		
	void			SetGlobalTransform(const Mat4& transform_mat);	
	inline Mat4		GetGlobalTransform() const		  { return global_transform_; }

	inline void		SetActorGroup(CollisionGroup msg) { group_msg_ = msg; }
	inline CollisionGroup
					GetActorGroup() const			  { return group_msg_;}
													  
	inline uint32_t GetActorID() const				  { return actor_id_; }
													  
	inline Model*	GetParent()  const				  { return parent_; }
	inline void		SetParent(Model* parent)		  { parent_ = parent; }

	void			AttachShape(AbstractShape& shape);
	//void			DetachShape(shared_ptr<AbstractShape> acotr_shape);

/// Fields
protected:
	_ShapePtrs		actor_shapes_		= {};
	Mat4			global_transform_	= Mat4::Identity();
	uint32_t		actor_id_			= -1;
	Model*			parent_				= nullptr;
	CollisionGroup	group_msg_			= {};


};

} // !namespace GComponent

#endif // !__RIGIDBODY_ACTOR_H
