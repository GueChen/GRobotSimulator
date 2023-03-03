#include "collider_component.h"
#include <algorithm>
namespace GComponent {

GComponent::ColliderComponent::ColliderComponent(Model* parent, _ShapeRawPtrs shapes) :
	Component(parent)
{
	shapes_.resize(shapes.size());
	for (int count = 0; auto & ptr : shapes) {
		shapes[count++] = ptr;
	}
}

void GComponent::ColliderComponent::RegisterShape(_ShapeRawPtr ptr)
{
	shapes_.push_back(_ShapePtr(ptr));
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

}