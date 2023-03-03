#include "component/component_factory.h"

#include "component/joint_component.h"
#include "component/joint_group_component.h"
#include "component/kinematic_component.h"
#include "component/tracker_component.h"

namespace GComponent{

std::unique_ptr<Component> ComponentFactory::GetComponent(const std::string& component_name, Model* parent_ptr)
{
    if (component_name == JointComponent::type_name) {
        return std::make_unique<JointComponent>(parent_ptr);
    }
    else if (component_name == JointGroupComponent::type_name) {
        return std::make_unique<JointGroupComponent>(parent_ptr);
    }
    else if (component_name == KinematicComponent::type_name) {
        return std::make_unique<KinematicComponent>(parent_ptr);
    }
    else if (component_name == TrackerComponent::type_name) {
        return std::make_unique<TrackerComponent>(parent_ptr);
    }
    return nullptr;
}


}