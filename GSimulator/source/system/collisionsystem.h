/**
 *  @file  	collisionsystem.h
 *  @brief 	This system used for custom collision detection usage.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Mar 13th, 2023
 **/
#ifndef __COLLISITON_SYSTEM_H
#define __COLLISITON_SYSTEM_H

#include "base/singleton.h"
#include "geometry/abstract_shape.hpp"

#include <QtCore/QObject>

#include <functional>
#include <tuple>
#include <list>
#include <map>

namespace GComponent {

struct CollisionMessage {
	uintptr_t m_belong = 0ull;	
};

class CollisionSystem : public QObject {
	Q_OBJECT

	NonCopyable(CollisionSystem)

public:
	using ShapePtrs		= std::vector<AbstractShape*>;
	using CRefShapePtrs = const ShapePtrs&;
	using Transform		= Eigen::Matrix4f;
	using CRefTransform = const Transform&;
	using CollisionFunc = std::function<bool(AbstractShape*, CRefTransform,
											 AbstractShape*, CRefTransform)>;

public:
	static CollisionSystem& getInstance();
	~CollisionSystem();
	void  Initialize();

	void  AddProcessShapes(CRefShapePtrs shapes, CRefTransform pose);

	void  tick(float delta_time);

protected:
	CollisionSystem() = default;

private:
	void OverlapCheck(CRefShapePtrs shapes_a, Transform pose_a, 
					  CRefShapePtrs shapes_b, Transform pose_b);

private:
	std::vector<std::pair<ShapePtrs, Transform>> need_process_;
	
/*____________________STATIC FIELDS____________________________________________*/	
private:
	static std::map<ShapeEnum, std::map<ShapeEnum, CollisionFunc>> collision_func_map;
};

}	// ! namespace GComponent

#endif // ! __COLLISITON_SYSTEM_H
