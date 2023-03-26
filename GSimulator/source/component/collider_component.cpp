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
	UpdateBoundingBox();
}

void ColliderComponent::DeregisterShape(_ShapeRawPtr ptr)
{
	std::erase_if(shapes_, [ptr = ptr](auto&& it_ptr) { return it_ptr.get() == ptr; });
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
		UpdateBoundingBox();
	}
	CollisionSystem::getInstance().AddProcessShapes(GetShapes(), cur_pose, ptr_parent_);
}

void ColliderComponent::UpdateBoundingBox()
{
	bound_ = BoundingBox::MergeTwoBoundingBox(bound_, boundings_.back());
}

}