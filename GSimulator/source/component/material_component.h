#ifndef __MATERIAL_COMPONENT_H
#define __MATERIAL_COMPONENT_H

#include "component/component.hpp"
#include "render/shader_property.hpp"
#include "function/conversion.hpp"

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
										 std::function<void(MyShader*, ShaderProperty&)>>;

public:
	explicit MaterialComponent(Model* ptr_parent) : Component(ptr_parent) {}
	MaterialComponent(Model* ptr_parent, std::string shader_name, bool cast_shadow): 
		Component(ptr_parent){
		cast_shadow_ = cast_shadow;
		SetShader(shader_name);
	}

	const std::string_view&
			 GetTypeName() const override final { return type_name; }	
	
	// Getters & Setters
	inline ShaderProperties& GetProperties()			{ return properties_; }

	void			   SetShader(const std::string& shader_name);

	inline std::string GetShader() const	    { return shader_;}

	// TODO: use reflect to replace this methods
	bool&  GetShadowCastRef()					{ return cast_shadow_; }
	inline bool		   GetIsCastShadow() const  { return cast_shadow_; }
	inline void		   SetIsCastShadow(bool val){ cast_shadow_ = val; }

	//	Renderer Invoke Inteface
	void  SetShaderProperties();


protected:
	void tickImpl(float delta) override;
	QJsonObject Save()				   override;

private:
	template<class T>
	static void SetFunction(MyShader* shader, ShaderProperty& var);

/// Fields
protected:
	//FIXME: not consider multiple pass situation, later consider how to solve it
	std::string			shader_;
	MyShader*			shader_ptr_	   = nullptr;
	ShaderProperties	properties_;
	bool				cast_shadow_   = true;

/// static Fields
public:
	constexpr static const std::string_view type_name = "MaterialComponent";

protected:
	//FIXME: not a good idea , if there is a type add or remove need refactor later
	static SetterMap setter_map;
	
};



}

#endif // !__MATERIAL_COMPONENT_H