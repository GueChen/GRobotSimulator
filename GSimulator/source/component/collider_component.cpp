#include "component/collider_component.h"

#include "base/json_serializer.hpp"

#include "system/collisionsystem.h"

#include "model/model.h"
#include "component/transform_component.h"

#include <algorithm>
#include <ranges>
#include <map>

#include <QtCore/QJsonArray>

namespace GComponent {

static std::map<ShapeEnum, std::function<QJsonObject(AbstractShape*)>>
shape_serializer = {
{Sphere,[](AbstractShape* shape_ptr)->QJsonObject {
	SphereShape* shape = static_cast<SphereShape*>(shape_ptr);
	QJsonObject json_obj;
	json_obj["type"]   = "Sphere";
	json_obj["radius"] = shape->m_radius;
	return json_obj;
}},
{Capsule,[](AbstractShape* shape_ptr)->QJsonObject {
	CapsuleShape* shape = static_cast<CapsuleShape*>(shape_ptr);
	QJsonObject json_obj;
	json_obj["type"]		= "Capsule";
	json_obj["radius"]	    = shape->m_radius;
	json_obj["half_height"] = shape->m_half_height;
	return json_obj;
}},
{Box,[](AbstractShape* shape_ptr)->QJsonObject {
	BoxShape* shape = static_cast<BoxShape*>(shape_ptr);
	QJsonObject json_obj;
	json_obj["type"]   = "Box";
	json_obj["half_x"] = shape->m_half_x;
	json_obj["half_y"] = shape->m_half_y;
	json_obj["half_z"] = shape->m_half_z;
	return json_obj;
}},
{ConvexHull,[](AbstractShape* shape_ptr)->QJsonObject {
	ConvexShape* shape = static_cast<ConvexShape*>(shape_ptr);
	QJsonObject json_obj;
	json_obj["type"] = "Convex";

	QJsonArray poses_obj;	
	for (auto& pos : shape->m_positions) {		
		poses_obj.append(JSonSerializer::Serialize(pos));
	}
	json_obj["positions"] = poses_obj;

	QJsonArray faces_obj;
	for (auto& face : shape->m_faces) {
		QJsonArray face;
		face.append(face[0]);
		face.append(face[1]);
		face.append(face[2]);
		faces_obj.append(face);
	}
	json_obj["faces"] = faces_obj;

	return json_obj;
}}
};

GComponent::ColliderComponent::ColliderComponent(Model* parent, _ShapeRawPtrs shapes) :
	Component(parent)
{	
	for (auto & ptr : shapes) {
		RegisterShape(ptr);
	}
}

void GComponent::ColliderComponent::RegisterShape(_ShapeRawPtr ptr)
{
	TransformCom& trans = *ptr_parent_->GetTransform();
	boundings_.push_back(BoundingBox::CompouteBoundingBox(*ptr, trans.GetTransGlobal(), Roderigues(trans.GetRotGlobal())));
	shapes_.push_back(_ShapePtr(ptr));
	UpdateBoundingBox(boundings_.back());
}

void ColliderComponent::DeregisterShape(_ShapeRawPtr ptr)
{
	auto s_iter = shapes_.begin(),    s_end = shapes_.end();
	auto b_iter = boundings_.begin(), b_end = boundings_.end();

	for (; s_iter != s_end && b_iter != b_end; ++s_iter, ++b_iter) 
	if (s_iter->get() == ptr) {			
		break;
	}

	shapes_.erase(s_iter);
	boundings_.erase(b_iter);	
}

ColliderComponent::_ShapeRawPtr ColliderComponent::GetShape(int idx)
{
	if (idx < 0 || idx >= shapes_.size()) return nullptr;
	return shapes_[idx].get();
}

ColliderComponent::_ShapeRawPtrs GComponent::ColliderComponent::GetShapes()
{
	_ShapeRawPtrs ret(shapes_.size());
	std::transform(shapes_.cbegin(), shapes_.cend(), ret.begin(), [](const auto& ptr) {return ptr.get(); });
	return ret;
}

const std::string_view& GComponent::ColliderComponent::GetTypeName() const
{
	return type_name;
}

/*_______________________________PROTECTED METHODS____________________________________________________________________*/
void ColliderComponent::tickImpl(float delta)
{	
#ifdef _COLLISION_TEST
	ptr_parent_->intesection_ = false;
#endif
	TransformCom& trans = *ptr_parent_->GetTransform();
	SE3f  cur_pose  = trans.GetModelMatrixWithoutScale();
	Vec3f cur_trans = cur_pose.block(0, 3, 3, 1);
	SO3f  cur_rot   = cur_pose.block(0, 0, 3, 3);
	if (trans.GetIsDirty()) {
		bound_ = BoundingBox();
		for (int i = 0; i < shapes_.size(); ++i) {
			boundings_[i] = BoundingBox::CompouteBoundingBox(*shapes_[i], cur_trans, cur_rot);
			UpdateBoundingBox(boundings_[i]);
		}
	}
	CollisionSystem::getInstance().AddBroadPhaseQuery(ptr_parent_, bound_);
	CollisionSystem::getInstance().AddProcessShapes(ptr_parent_, GetShapes(), cur_pose, is_static_);
}

void ColliderComponent::UpdateBoundingBox(const BoundingBox& box)
{
	bound_ = BoundingBox::MergeTwoBoundingBox(bound_, box);
}

/*_______________________________SAVE METHODS_________________________________________________________________*/
static QJsonObject SerializeBoundingBox(const BoundingBox& bound) {
	QJsonObject obj;
	obj["min"] = JSonSerializer::Serialize(bound.m_min);
	obj["max"] = JSonSerializer::Serialize(bound.m_max);
	return obj;
}

QJsonObject ColliderComponent::Save()
{
	QJsonObject com_obj;
	com_obj["type"] = QString(type_name.data());
	
	QJsonArray shapes_obj;
	for (auto& shape : shapes_) {
		shapes_obj.append(shape_serializer[shape->GetShapeType()](shape.get()));
	}
	com_obj["shapes"] = shapes_obj;

	QJsonArray bounds_obj;
	for (auto& bound : boundings_) {
		bounds_obj.append(SerializeBoundingBox(bound));
	}
	com_obj["boundings"] = bounds_obj;

	com_obj["bound"] = SerializeBoundingBox(bound_);
	
	com_obj["is_static"] = is_static_;

	return com_obj;
}

}