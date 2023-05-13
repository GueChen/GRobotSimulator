/**
 *  @file  	rigidbody_component.h
 *  @brief 	This class is used for an abstract about physics rigidbody.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 8th, 2022
 **/
#ifndef __RIGIDBODY_COMPONENT_H
#define __RIGIDBODY_COMPONENT_H

#include "component/component.hpp"

#include "geometry/abstract_shape.hpp"
#include "physics/rigidbody_actor.h"

#include <string>
#include <tuple>

namespace GComponent {

//TODO: complete this class
class RigidbodyComponent : public Component {
// 1. Support Physics Update acoording the elapsed time
// 2. Support mass and innertial computation from triangle mesh or particle system
// 3. Physics Material properties Setter and Getter
public:
	/// TODO: consider whether exsiting a better solution for the constructor ?
	// Sphere  Version
	explicit		RigidbodyComponent(Model* ptr_parent, const Mat4& local_mat, float radius, CollisionGroup group = {});
	// Capsule Version
	explicit		RigidbodyComponent(Model* ptr_parent, const Mat4& local_mat, float radius, float half_height, CollisionGroup group = {});
	// Box	   Version
	explicit		RigidbodyComponent(Model* ptr_parent, const Mat4& local_mat, float half_x, float half_y, float half_z, CollisionGroup group = {});
	
	~RigidbodyComponent();
	
	virtual const string_view& 
					GetTypeName() const override{ return type_name; };
	inline RigidBodyActor& 
					GetActor()	  const			{ return *rigidbody_actor_; }
	inline Mat4		GetLocalModelMat() const	{ return local_model_matrix_; }

protected:
	void			tickImpl(float delta_time) override;
	QJsonObject		Save() override;

private:
	RigidBodyActor* CreateRigidBodyActorFromScene(AbstractShape& shape, CollisionGroup group);

protected:
	Mat4				   local_model_matrix_		  = Mat4::Identity();
	RigidBodyActor*		   rigidbody_actor_			  = nullptr;

public:
	constexpr static const std::string_view type_name = "RigidbodyComponent";
};

} //  !namespace GComponent

#endif // !__RIGIDBODY_COMPONENT_H
