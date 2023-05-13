#include "component/component_factory.h"
#include "component/Components.h"

namespace GComponent{

std::map<std::string_view, ComponentFactory::CreateFunc> 
ComponentFactory::builder = {
#define BuilderPair(Type) \
{Type::type_name, [](Model* model){ return std::make_unique<Type>(model); }}

BuilderPair(ColliderComponent),
BuilderPair(JointComponent),
BuilderPair(JointGroupComponent),
BuilderPair(KinematicComponent),
BuilderPair(MaterialComponent),
BuilderPair(RigidbodyComponent),
BuilderPair(TrackerComponent),
BuilderPair(TransformComponent)

#undef BuilderPair
};

}