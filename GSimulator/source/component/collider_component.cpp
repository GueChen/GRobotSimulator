#include "component/collider_component.h"

#include "system/collisionsystem.h"

#include "model/model.h"

#include <algorithm>
#include <ranges>

namespace GComponent {

GComponent::ColliderComponent::ColliderComponent(Model* parent, _ShapeRawPtrs shapes) :
	Component(parent)
{	
	for (auto & ptr : shapes) {
		RegisterShape(ptr);
	}
}

void GComponent::ColliderComponent::RegisterShape(_ShapeRawPtr ptr)
{
	boundings_.push_back(BoundingBox::CompouteBoundingBox(*ptr, ptr_parent_->getTransGlobal(), Roderigues(ptr_parent_->getRotGlobal())));
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
	bound_ = BoundingBox();
	SE3f  cur_pose  = ptr_parent_->getModelMatrixWithoutScale();
	Vec3f cur_trans = cur_pose.block(0, 3, 3, 1);
	SO3f  cur_rot   = cur_pose.block(0, 0, 3, 3);
	for (int i = 0; i < shapes_.size(); ++i) {
		boundings_[i] = BoundingBox::CompouteBoundingBox(*shapes_[i], cur_trans, cur_rot);
		UpdateBoundingBox(boundings_[i]);
	}

	CollisionSystem::getInstance().AddBroadPhaseQuery(ptr_parent_, bound_);
	CollisionSystem::getInstance().AddProcessShapes(ptr_parent_, GetShapes(), cur_pose);
}

void ColliderComponent::UpdateBoundingBox(const BoundingBox& box)
{
	bound_ = BoundingBox::MergeTwoBoundingBox(bound_, box);
}

}