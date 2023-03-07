#include "component/material_component.h"

#include "render/myshader.h"

#include "manager/rendermanager.h"
#include "manager/resourcemanager.h"

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
		GlmSetterPair(mat4)
};

void GComponent::MaterialComponent::SetShader(const std::string& shader_name)
{
	shader_ = shader_name;
	auto& render = RenderManager::getInstance();
	auto& resource = ResourceManager::getInstance();
	MyShader* shader_ptr = resource.GetShaderByName(shader_name);
	properties_ = shader_ptr->GetProperties();
}

void GComponent::MaterialComponent::SetShaderProperties() const
{
	MyShader* shader = ResourceManager::getInstance().GetShaderByName(shader_);
	if (!shader) return;
	shader->use();
	for (auto&& var : properties_) {
		setter_map[var.type](shader, var);
	}
}
}