#include "component/material_component.h"

#include "render/myshader.h"
#include "manager/resourcemanager.h"

#include "model/model.h"

namespace GComponent {

#define SetterPair(Type)	std::make_pair(#Type, MaterialComponent::SetFunction<Type>)
#define GlmSetterPair(Type) std::make_pair(#Type, MaterialComponent::SetFunction<glm::Type>)

template<class T>
void MaterialComponent::SetFunction(MyShader* shader, Property& var)
{
	assert(shader && "In MaterialComponent::SetFunction, shader ptr can't be nullptr\n");
	shader->setUniformValue(var.location, std::get<T>(var.val));
}

MaterialComponent::SetterMap MaterialComponent::setter_map = {
		SetterPair	 (int),
		SetterPair   (unsigned int),
		SetterPair	 (bool),
		SetterPair	 (float),			
		GlmSetterPair(vec2),
		GlmSetterPair(vec3),
		GlmSetterPair(vec4),
		GlmSetterPair(mat4),
		{"sampler2D",	   MaterialComponent::SetFunction<int>},
		{"sampler2DArray", MaterialComponent::SetFunction<int>}
};

void GComponent::MaterialComponent::SetShader(const std::string& shader_name)
{
	shader_ = shader_name;	
	auto& resource = ResourceManager::getInstance();
	shader_ptr_ = resource.GetShaderByName(shader_name);
	if (!shader_ptr_) return;
	properties_ = shader_ptr_->GetProperties();
}

void GComponent::MaterialComponent::SetShaderProperties()
{
	if (!shader_ptr_) SetShader(shader_);	
	if (!shader_ptr_) return;
	shader_ptr_->use();
	for (auto&& var : properties_) {
		if (var.name == "model") var.val = Conversion::fromMat4f(GetParent()->getModelMatrix());
		setter_map[var.type](shader_ptr_, var);
	}
}
}