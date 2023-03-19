/**
 *  @file  	physx_utils.h
 *  @brief 	This file is some physx type wrappper.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 7th, 2022
 **/
#ifndef __PHYSX_UTILS_H
#define __PHYSX_UTILS_H

#include "geometry/abstract_shape.hpp"

#include <Eigen/Dense>
#include <foundation/PxVec4.h>
#include <foundation/PxVec3.h>
#include <foundation/PxMat44.h>
#include <foundation/PxMat33.h>
#include <geometry/PxCapsuleGeometry.h>
#include <geometry/PxSphereGeometry.h>
#include <geometry/PxBoxGeometry.h>

#include <memory>

namespace GComponent {

class PhysXUtils{
public:
	PhysXUtils() = delete;

	static inline physx::PxVec3		fromVec3f(const Eigen::Vector3f& vec) { return physx::PxVec3(vec.x(), vec.y(), vec.z()); }
	static inline Eigen::Vector3f	toVec3f(const   physx::PxVec3  & vec) { return Eigen::Vector3f(vec.x, vec.y, vec.z); }

	static inline physx::PxVec4		fromVec4f(const Eigen::Vector4f& vec) { return physx::PxVec4(vec.x(), vec.y(), vec.z(), vec.w()); }
	static inline Eigen::Vector4f	toVec4f(const	physx::PxVec4  & vec) { return Eigen::Vector4f(vec.x, vec.y, vec.z, vec.w); }

	static physx::PxMat44			fromMat4f(const Eigen::Matrix4f& mat);
	static Eigen::Matrix4f			toMat4f(const   physx::PxMat44 & mat);

	static physx::PxMat33			formMat3f(const Eigen::Matrix3f& mat);
	static Eigen::Matrix3f			toMat3f(const   physx::PxMat33 & mat);

	static std::shared_ptr<physx::PxGeometry>		
									fromShape(AbstractShape& shape);
	//static AbstractShape*			toShape(physx::PxGeometry& geometry);

};

}	// !namespace GComponent

#endif // !__PHYSX_UTILS_H