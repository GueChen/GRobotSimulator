#include "component/material_component.h"

#include "render/myshader.h"
#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"

#include "model/model.h"
#include "component/transform_component.h"

#include "base/json_serializer.hpp"

#include <QtCore/QJsonArray>

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
		if (var.name == "model") {
			TransformCom* trans = GetParent()->GetTransform();
			var.val = Conversion::fromMat4f(trans->GetModelGlobal());
		}
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

/*___________________________________________________Save Methods___________________________________________________*/
//FIXME: bad practice with too much manual work
//TODO: add reflect system making better suitable
static QJsonValue FromVar(ShaderProperty& var) {
	if (var.type == "int") {
		return std::get<int>(var.val);
	}
	else if (var.type == "unsigned int") {
		return static_cast<int>(std::get<unsigned int>(var.val));
	}
	else if (var.type == "bool") {
		return std::get<bool>(var.val);
	}
	else if (var.type == "float") {
		return std::get<float>(var.val);
	}
	else if (var.type == "vec2") {
		vec2 value = std::get<vec2>(var.val);
		QJsonArray array_obj;
		array_obj.append(value[0]);
		array_obj.append(value[1]);
		return array_obj;
	}
	else if (var.type == "vec3") {
		vec3 value = std::get<vec3>(var.val);
		QJsonArray array_obj;
		array_obj.append(value[0]);
		array_obj.append(value[1]);
		array_obj.append(value[2]);
		return array_obj;
	}
	else if (var.type == "vec4") {
		vec4 value = std::get<vec4>(var.val);
		QJsonArray array_obj;
		array_obj.append(value[0]);
		array_obj.append(value[1]);
		array_obj.append(value[2]);
		array_obj.append(value[3]);
		return array_obj;
	}
	else if (var.type == "mat4") {
		Eigen::Matrix4f value = Conversion::toMat4f(std::get<mat4>(var.val));
		return JSonSerializer::Serialize(value);
	}
	else if (var.type == "sampler2D") {
		return static_cast<int>(std::get<Texture>(var.val).id);
	}
	else if (var.type == "sampler2DArray") {
		return static_cast<int>(std::get<Texture>(var.val).id);
	}
	else if (var.type == "samplerCUBE") {
		return static_cast<int>(std::get<Texture>(var.val).id);
	}
	else if (var.type == "color") {
		vec3 value = std::get<vec3>(var.val);
		QJsonArray array_obj;
		array_obj.append(value[0]);
		array_obj.append(value[1]);
		array_obj.append(value[2]);
		return array_obj;
	}
	else {
		assert(false && "unexpected type occur");
	}	
}

static ShaderProperty::Var ToPropertyVar(const QJsonObject& obj) {
	ShaderProperty::Var var;
	std::string type = obj["type"].toString().toStdString();
	QJsonValue  val = obj["val"];
	if (type == "int") {
		var = val.toInt();
	}
	else if (type == "unsigned int") {
		var = val.toInt();
	}
	else if (type == "bool") {
		var = val.toBool();
	}
	else if (type == "float") {
		var = static_cast<float>(val.toDouble());
	}
	else if (type == "vec2") {
		QJsonArray vec_obj = val.toArray();
		var = glm::vec2(vec_obj[0].toDouble(), 
						vec_obj[1].toDouble());
	}
	else if (type == "vec3") {
		QJsonArray vec_obj = val.toArray();
		var = glm::vec3(vec_obj[0].toDouble(),
						vec_obj[1].toDouble(),
						vec_obj[2].toDouble());
	}
	else if (type == "vec4") {
		QJsonArray vec_obj = val.toArray();
		var = glm::vec4(vec_obj[0].toDouble(),
						vec_obj[1].toDouble(),
						vec_obj[2].toDouble(),
						vec_obj[3].toDouble());
	}
	else if (type == "mat4") {		
		Eigen::Matrix4f mat = JsonDeserializer::ToMat4f(val.toArray());
		var = Conversion::fromMat4f(mat);
	}
	else if (type == "sampler2D") {
		Texture texture;
		texture.id = val.toInt();
		var = texture;
	}
	else if (type == "sampler2DArray") {
		Texture texture;
		texture.id = val.toInt();
		var = texture;
	}
	else if (type == "samplerCUBE") {
		Texture texture;
		texture.id = val.toInt();
		var = texture;
	}
	else if (type == "color") {
		QJsonArray vec_obj = val.toArray();
		var = glm::vec3(vec_obj[0].toDouble(),
						vec_obj[1].toDouble(),
						vec_obj[2].toDouble());	
	}
	else {
		assert(false && "unexpected type occur");
	}
	return var;
}

QJsonObject MaterialComponent::Save()
{
	QJsonObject com_obj;
	
	com_obj["type"] = type_name.data();
	com_obj["shader"] = shader_.data();
	
	QJsonArray properties;
	for (auto& prop : properties_) {
		QJsonObject prop_obj;
		prop_obj["type"] = prop.type.data();
		prop_obj["name"] = prop.name.data();
		prop_obj["val"]  = FromVar(prop);
		properties.append(prop_obj);
	}
	com_obj["properties"]  = properties;
	com_obj["cast_shadow"] = cast_shadow_;

	return com_obj;
}
bool MaterialComponent::Load(const QJsonObject& com_obj)
{
	shader_ = com_obj["shader"].toString().toStdString();
	// Get property lists
	SetShader(shader_);

	std::unordered_map<std::string, ShaderProperty::Var> var_map;
	// transfer json to var
	QJsonArray properties_obj = com_obj["properties"].toArray();
	for (const QJsonValue& json_val : properties_obj) {
		QJsonObject prop_obj = json_val.toObject();		
		std::string name = prop_obj["name"].toString().toStdString();
		var_map[name] = ToPropertyVar(prop_obj);
	}

	for (auto& prop : properties_) {
		prop.val = var_map[prop.name];
	}

	cast_shadow_ = com_obj["cast_shadow"].toBool();

	return true;
}
}