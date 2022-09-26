#include "physics/physx/physx_utils.h"


namespace GComponent {
	physx::PxMat44 PhysXUtils::fromMat4f(const Eigen::Matrix4f& mat)
	{
		return physx::PxMat44(fromVec4f(mat.col(0)), 
							  fromVec4f(mat.col(1)), 
							  fromVec4f(mat.col(2)), 
							  fromVec4f(mat.col(3)));
	}

	Eigen::Matrix4f PhysXUtils::toMat4f(const physx::PxMat44& mat)
	{
		return Eigen::Matrix4f{ {mat.column0.x, mat.column1.x, mat.column2.x, mat.column3.x},
								{mat.column0.y, mat.column1.y, mat.column2.y, mat.column3.y},
								{mat.column0.z, mat.column1.z, mat.column2.z, mat.column3.z}, 
								{mat.column0.w, mat.column1.w, mat.column2.w, mat.column3.w} };
	}

	physx::PxMat33 PhysXUtils::formMat3f(const Eigen::Matrix3f& mat)
	{
		return physx::PxMat33(fromVec3f(mat.col(0)),
							  fromVec3f(mat.col(1)),
							  fromVec3f(mat.col(2)));
	}

	Eigen::Matrix3f PhysXUtils::toMat3f(const physx::PxMat33& mat)
	{
		return Eigen::Matrix3f{{mat.column0.x, mat.column1.x, mat.column2.x},
							   {mat.column0.y, mat.column1.y, mat.column2.y},
							   {mat.column0.z, mat.column1.z, mat.column2.z}};
	}

	std::shared_ptr<physx::PxGeometry> PhysXUtils::fromShape(AbstractShape& shape)
	{
		std::shared_ptr<physx::PxGeometry> px_geometry = nullptr;
		switch (shape.GetShapeType()) {
		using enum ShapeEnum;
		case Capsule: {
			CapsuleShape& raw_capsule	= dynamic_cast<CapsuleShape&>(shape);
			px_geometry					= std::make_shared<physx::PxCapsuleGeometry>(raw_capsule.m_radius, raw_capsule.m_half_height);
			break;
		}
		case Sphere:{
			SphereShape&  raw_sphere	= dynamic_cast<SphereShape&>(shape);
			px_geometry					= std::make_shared<physx::PxSphereGeometry>(raw_sphere.m_radius);
			break;
		}
		case Box:{
			BoxShape&	  raw_box		= dynamic_cast<BoxShape&>(shape);
			px_geometry					= std::make_shared<physx::PxBoxGeometry>(raw_box.m_half_x, raw_box.m_half_y, raw_box.m_half_z);
			break;
		}
		}	
		return px_geometry;
	}

} // !namespace GComponent