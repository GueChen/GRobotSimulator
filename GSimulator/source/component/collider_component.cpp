#include "component/collider_component.h"

#include "system/collisionsystem.h"

#include "model/model.h"

#include <algorithm>

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
	shapes_.push_back(_ShapePtr(ptr));
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

void ColliderComponent::tickImpl(float delta)
{	
#ifdef _COLLISION_TEST
	ptr_parent_->intesection_ = false;
#endif
	CollisionSystem::getInstance().AddProcessShapes(GetShapes(), ptr_parent_->getModelMatrixWithoutScale(), ptr_parent_);
}

}