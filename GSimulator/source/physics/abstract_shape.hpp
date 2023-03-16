/**
 *  @file  	abstract_shape.hpp
 *  @brief 	This is a base abstract class implement some interface to implement for derived class.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Aug 9th, 2022
 **/
#ifndef __ABSTRACT_SHAPE_HPP
#define __ABSTRACT_SHAPE_HPP

#include "physics/physics_datastructure.hpp"
#include "render/rendering_datastructure.hpp"

#include <Eigen/Dense>
#include <vector>

namespace GComponent {

enum class ShapeEnum {
	Sphere		= 0,
	Capsule		= 1,
	Box			= 2,
	Plane		= 3,
	ConvexHull	= 4,
	None		= -1
};

class AbstractShape {
public:
	AbstractShape()				= default;
	virtual ~AbstractShape()	= default;
	///AbstractShape(const AbstractShape&) = default;
public:	
	virtual ShapeEnum GetShapeType() const = 0;
};

class SphereShape : public AbstractShape {
public:
	SphereShape(float radius) : m_radius(radius) {}
	~SphereShape()	= default;

	virtual ShapeEnum GetShapeType() const override { return m_type_enum; }
public:
	float				m_radius		= 0.0f;
	static const ShapeEnum		
						m_type_enum		= ShapeEnum::Sphere;
};

class CapsuleShape : public AbstractShape{
public:
	CapsuleShape(float radius, float half_height) : m_radius(radius), m_half_height(half_height) {}
	~CapsuleShape() = default;
	virtual ShapeEnum GetShapeType() const override { return m_type_enum; }
public:
	float				m_radius		= 0.0f;
	float				m_half_height	= 0.0f;
	static const ShapeEnum		
						m_type_enum		= ShapeEnum::Capsule;
};

class BoxShape : public AbstractShape {
public:
	BoxShape(float half_x, float half_y, float half_z): m_half_x(half_x), m_half_y(half_y), m_half_z(half_z) {}
	~BoxShape() = default;
	virtual ShapeEnum GetShapeType() const override { return m_type_enum; }
public:
	float				m_half_x		= 0.0f;
	float				m_half_y		= 0.0f;
	float				m_half_z		= 0.0f;
	static const ShapeEnum		
						m_type_enum		= ShapeEnum::Box;
};

class PlaneShape : public AbstractShape {
public:
	PlaneShape() { 
		// No Implementation
		assert(false); 
	}

public:
	Eigen::Vector3f		n;// normal of the plane
	float				d;// distance from the origin
	static const ShapeEnum
						m_type_enum		= ShapeEnum::Plane;
};

class ConvexShape : public AbstractShape {
public:
	struct Line {
		int first;
		int second;
	};
	struct Facelet {
		
	};

	ConvexShape() {
		// No Implementation
		assert(false);
	}
public:
	
	std::vector<Eigen::Vector3f> m_positions;	
	std::vector<Triangle>	     m_faces;
	static const ShapeEnum		 m_type_enum = ShapeEnum::ConvexHull;
};

} // !namespace GComponent

#endif // !__ABSTRACT_SHAPE_HPP

