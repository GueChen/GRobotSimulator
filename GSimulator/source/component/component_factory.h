#ifndef __COMPONENT_FACTORY_H
#define __COMPONENT_FACTORY_H

#include "component/component.hpp"

#include <memory>
#include <functional>
#include <string>
#include <map>

namespace GComponent {

class Model;

class ComponentFactory
{
public:
	using CreateFunc = std::function<std::unique_ptr<Component>(Model* model)>;
	ComponentFactory() = delete;

	static std::map<std::string_view, CreateFunc> builder;
};
}
#endif // !__COMPONENT_FACTORY_H



