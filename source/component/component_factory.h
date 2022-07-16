#ifndef __COMPONENT_FACTORY_H
#define __COMPONENT_FACTORY_H

#include "component/component.hpp"

#include <memory>

namespace GComponent {

class Model;

class ComponentFactory
{
public:
	static std::unique_ptr<Component> GetComponent(const std::string& component_name, Model* parent_ptr);

	ComponentFactory() = delete;
};
}
#endif // !__COMPONENT_FACTORY_H



