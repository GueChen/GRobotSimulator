#include "component/material_component.h"

#include "render/myshader.h"
#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"

#include "model/model.h"

namespace GComponent {

#define SetterPair(Type)	std::make_pair(#Type, MaterialComponent::SetFunction<Type>)
#define GlmSetterPair(Type) std::make_pair(#Type, MaterialComponent::SetFunction<glm::Type>)

template<class T>
void MaterialComponent::SetFunction(MyShader* shader, ShaderProperty& var)
{
	assert(shader && "In MaterialComponent::SetFunction, shader ptr can't be nullptr\n");
	shader->setUniformValue(var.location, std::get<T>(var.val));
}

template<>
void MaterialComponent::SetFunction<Texture>(MyShader* shader, ShaderProperty& var)
{
	assert(shader && "In MaterialComponent::SetFunction, shader ptr can't be nullptr\n");
	shader->setUniformValue(var.location, std::get<Texture>(var.val).id);
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
		{"sampler2D",	   MaterialComponent::SetFunction<Texture>},
		{"sampler2DArray", MaterialComponent::SetFunction<Texture>},
		{"samplerCUBE",	   MaterialComponent::SetFunction<Texture>},
		{"color",		   MaterialComponent::SetFunction<glm::vec3>}
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
	shader_ptr_ = ResourceManager::getInstance().GetShaderByName(shader_);
	if (!shader_ptr_) {
		shader_ = "null";
		properties_.clear();
		return;
	}
	shader_ptr_->use();
	for (auto&& var : properties_) {
		if (var.name == "model") var.val = Conversion::fromMat4f(GetParent()->getModelMatrix());
		setter_map[var.type](shader_ptr_, var);
	}
}
void MaterialComponent::tickImpl(float delta)
{
	auto& renderer = RenderManager::getInstance();
	if (shader_ == "axis") {
		renderer.EmplaceRenderCommand(ptr_parent_->getName(), ptr_parent_->getMesh(), RenderManager::PostProcess);
	}
	else {
		renderer.EmplaceRenderCommand(ptr_parent_->getName(), ptr_parent_->getMesh(), RenderManager::Normal);
	}

	if (cast_shadow_) {
		renderer.EmplaceRenderCommand(ptr_parent_->getName(), ptr_parent_->getMesh(), RenderManager::Depth);
	}
}
}