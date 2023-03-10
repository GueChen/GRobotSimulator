#ifndef __MATERIAL_COMPONENT_H
#define __MATERIAL_COMPONENT_H

#include "component/component.hpp"
#include "render/shader_property.hpp"

#include <unordered_map>
#include <string_view>
#include <functional>
#include <string>
#include <cassert>

namespace GComponent{

class MyShader;

class MaterialComponent : public Component {
public:
	friend class RenderManager;

protected:
	using SetterMap = std::unordered_map<std::string, 
										 std::function<void(MyShader*, Property&)>>;

public:
	explicit MaterialComponent(Model* ptr_parent) : Component(ptr_parent) {}

	const std::string_view&
			 GetTypeName() const override final { return type_name; }	
	
	// Getters & Setters
	inline Properties& GetProperties()			{ return properties_; }

	void			   SetShader(const std::string& shader_name);

	inline std::string GetShader() const	    { return shader_;}

	//	Renderer Invoke Inteface
	void  SetShaderProperties() const;

private:
	template<class T>
	static void SetFunction(MyShader* shader, Property& var);

/// Fields
protected:
	//FIXME: not consider multiple pass situation, later consider how to solve it
	std::string			shader_;
	mutable Properties	properties_;

/// static Fields
public:
	constexpr static const std::string_view type_name = "MaterialComponent";

protected:
	//FIXME: not a good idea , if there is a type add or remove need refactor later
	static SetterMap setter_map;
	
};



}

#endif // !__MATERIAL_COMPONENT_H