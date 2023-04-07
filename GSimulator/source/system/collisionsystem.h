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
#include "geometry/bounding_box.h"

#include <QtCore/QObject>

#include <functional>
#include <tuple>
#include <list>
#include <map>

namespace GComponent {

class Model;

struct CollisionMessage {
	uintptr_t m_belong = 0ull;	
};

class CollisionSystem : public QObject {
	Q_OBJECT

	NonCopyable(CollisionSystem)
public:
	static CollisionSystem& getInstance();
	~CollisionSystem();
	void  Initialize();

	void  AddProcessShapes  (Model* key, CRefShapePtrs shapes, CRefTransform pose);
	void  AddBroadPhaseQuery(Model* key, const BoundingBox& box);
	void  tick(float delta_time);

protected:
	CollisionSystem() = default;

private:
	bool OverlapCheck(CRefShapePtrs shapes_a, Transform pose_a, 
					  CRefShapePtrs shapes_b, Transform pose_b);
	void BroadPhaseQuery();

private:
	std::unordered_map<Model*, std::pair<ShapePtrs, Transform>> shape_table_;
	std::vector<std::pair<Model*, Model*>>						narrow_need_process_;
	std::vector<std::pair<Model*, BoundingBox>>					broad_need_process_;

};

}	// ! namespace GComponent

#endif // ! __COLLISITON_SYSTEM_H
