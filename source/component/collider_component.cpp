#include "collider_component.h"

const std::string_view& GComponent::ColliderComponent::GetTypeName() const
{
	return type_name;
}
