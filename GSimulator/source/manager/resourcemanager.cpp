#include "manager/resourcemanager.h"

#include <QtCore/QMetaType>
#include <algorithm>
#include <iostream>
#include <format>
#include <type_traits>

namespace GComponent {
	using std::move;

	namespace {

	std::string NormalizeShaderPropertyName(std::string_view name)
	{
		std::string normalized(name);
		std::replace(normalized.begin(), normalized.end(), '_', ' ');
		return normalized;
	}

	const RhiMaterialParameterDesc* FindParameterDesc(const RhiMaterialDesc& material_desc, std::string_view property_name)
	{
		const std::string normalized_name = NormalizeShaderPropertyName(property_name);
		for (const auto& parameter : material_desc.parameters) {
			if (parameter.name == normalized_name) {
				return &parameter;
			}
		}
		return nullptr;
	}

	std::string GetShaderPropertyTypeName(const ShaderProperty::Var& value)
	{
		return std::visit([](const auto& typed_value) -> std::string {
			using ValueType = std::decay_t<decltype(typed_value)>;
			if constexpr (std::is_same_v<ValueType, bool>) {
				return "bool";
			}
			else if constexpr (std::is_same_v<ValueType, int>) {
				return "int";
			}
			else if constexpr (std::is_same_v<ValueType, unsigned int>) {
				return "unsigned int";
			}
			else if constexpr (std::is_same_v<ValueType, float>) {
				return "float";
			}
			else if constexpr (std::is_same_v<ValueType, double>) {
				return "double";
			}
			else if constexpr (std::is_same_v<ValueType, glm::vec2>) {
				return "vec2";
			}
			else if constexpr (std::is_same_v<ValueType, glm::vec3>) {
				return "vec3";
			}
			else if constexpr (std::is_same_v<ValueType, Color>) {
				return "color";
			}
			else if constexpr (std::is_same_v<ValueType, glm::vec4>) {
				return "vec4";
			}
			else if constexpr (std::is_same_v<ValueType, glm::mat2>) {
				return "mat2";
			}
			else if constexpr (std::is_same_v<ValueType, glm::mat3>) {
				return "mat3";
			}
			else if constexpr (std::is_same_v<ValueType, glm::mat4>) {
				return "mat4";
			}
			else if constexpr (std::is_same_v<ValueType, Texture>) {
				return "sampler2D";
			}
			else {
				static_assert(!sizeof(ValueType*), "unsupported ShaderProperty::Var type");
			}
		}, value);
	}

	bool ApplyOpenGLShaderProperty(MyShader& shader, const ShaderProperty& property)
	{
		if (property.location >= 0) {
			return std::visit([&shader, location = property.location](const auto& typed_value) -> bool {
				using ValueType = std::decay_t<decltype(typed_value)>;
				if constexpr (std::is_same_v<ValueType, bool>
					|| std::is_same_v<ValueType, int>
					|| std::is_same_v<ValueType, unsigned int>
					|| std::is_same_v<ValueType, float>
					|| std::is_same_v<ValueType, glm::vec2>
					|| std::is_same_v<ValueType, glm::vec3>
					|| std::is_same_v<ValueType, glm::vec4>
					|| std::is_same_v<ValueType, glm::mat4>) {
					shader.setUniformValue(location, typed_value);
					return true;
				}
				else if constexpr (std::is_same_v<ValueType, Texture>) {
					shader.setUniformValue(location, typed_value.id);
					return true;
				}
				else if constexpr (std::is_same_v<ValueType, Color>) {
					shader.setUniformValue(location, typed_value.val);
					return true;
				}
				else {
					return false;
				}
			}, property.val);
		}

		return std::visit([&shader, &property](const auto& typed_value) -> bool {
			using ValueType = std::decay_t<decltype(typed_value)>;
			if constexpr (std::is_same_v<ValueType, bool>) {
				shader.setBool(property.name, typed_value);
				return true;
			}
			else if constexpr (std::is_same_v<ValueType, int>) {
				shader.setInt(property.name, typed_value);
				return true;
			}
			else if constexpr (std::is_same_v<ValueType, unsigned int>) {
				shader.setUint(property.name, typed_value);
				return true;
			}
			else if constexpr (std::is_same_v<ValueType, float>) {
				shader.setFloat(property.name, typed_value);
				return true;
			}
			else if constexpr (std::is_same_v<ValueType, glm::vec3>) {
				shader.setVec3(property.name, typed_value);
				return true;
			}
			else if constexpr (std::is_same_v<ValueType, glm::mat4>) {
				shader.setMat4(property.name, typed_value);
				return true;
			}
			else if constexpr (std::is_same_v<ValueType, Color>) {
				shader.setVec3(property.name, typed_value.val);
				return true;
			}
			else if constexpr (std::is_same_v<ValueType, Texture>) {
				shader.setInt(property.name, static_cast<int>(typed_value.id));
				return true;
			}
			else {
				return false;
			}
		}, property.val);
	}

	}

	ResourceManager::ResourceManager() {

	}

	ResourceManager& ResourceManager::getInstance()
	{
		static ResourceManager instance;
		return instance;
	}

	ResourceManager::~ResourceManager() = default;

	void ResourceManager::EnablePickingMode()
	{
		RegisteredShader(MakeOpenGlShaderDesc("picking", PathVert(picking), PathFrag(picking)));
	}

	void ResourceManager::RegisteredMesh(const string& name, RenderMesh* raw_ptr_mesh) 
	{
		if (!raw_ptr_mesh) {
			std::cerr << "RegisteredMesh failed: mesh is nullptr, name = " << name << '\n';
			return;
		}

		DeregisteredMesh(name);
		mesh_require_upload_.push_back(name);
		mesh_map_.emplace(name, unique_ptr<RenderMesh>(raw_ptr_mesh));
	}

	RenderMesh* ResourceManager::GetMeshByName(const string& name) {
		auto iter = mesh_map_.find(name);
		if (iter != mesh_map_.end()) {
			return iter->second.get();
		}
		return nullptr;
	}

	void ResourceManager::RegisteredShader(const RhiShaderDesc& shader_desc, QObject* parent)
	{
		(void)parent;
		if (shader_desc.name.empty()) {
			std::cerr << "RegisteredShader failed: shader name is empty\n";
			return;
		}
		if (!shader_desc.vertex.IsValid() || !shader_desc.fragment.IsValid()) {
			std::cerr << "RegisteredShader failed: vertex or fragment stage missing, name = " << shader_desc.name << '\n';
			return;
		}

		DeregisteredShader(shader_desc.name);
		shader_require_upload_.push_back(shader_desc.name);
		shader_registry_.emplace(shader_desc.name, ShaderRegistryEntry{
			.shader_desc = shader_desc,
			.material_desc = RhiMaterialDesc{
				.shader_name = shader_desc.name,
				.backend = shader_desc.backend
			}
		});
	}

	void ResourceManager::RegisteredShader(const string& name, MyShader* raw_ptr_shader)
	{
		if (!raw_ptr_shader) {
			std::cerr << "RegisteredShader failed: shader is nullptr, name = " << name << '\n';
			return;
		}
		DeregisteredShader(name);
		shader_require_upload_.push_back(name);
		raw_ptr_shader->SetName(name);
		auto& entry = shader_registry_[name];
		entry.shader_desc = raw_ptr_shader->GetShaderDesc();
		entry.shader_desc.name = name;
		entry.material_desc = raw_ptr_shader->GetMaterialDesc();
		entry.material_desc.shader_name = name;
		entry.opengl_shader.reset(raw_ptr_shader);
	}

	void ResourceManager::DeregisteredShader(const string& name)
	{
		DeregisteredSpecificMapElement(shader_registry_, name);
	}

	bool ResourceManager::HasShader(const string& name) const
	{
		return shader_registry_.find(name) != shader_registry_.end();
	}

	MyShader* ResourceManager::GetShaderByName(const string& name)
	{
		auto iter = shader_registry_.find(name);
		if (iter != shader_registry_.end()) {
			return iter->second.opengl_shader.get();
		}
		return nullptr;
	}

	const RhiShaderDesc* ResourceManager::GetShaderDescByName(const string& name) const
	{
		auto iter = shader_registry_.find(name);
		if (iter != shader_registry_.end()) {
			return &iter->second.shader_desc;
		}
		return nullptr;
	}

	const RhiMaterialDesc* ResourceManager::GetMaterialDescByShaderName(const string& name) const
	{
		auto iter = shader_registry_.find(name);
		if (iter != shader_registry_.end()) {
			return &iter->second.material_desc;
		}
		return nullptr;
	}

	std::vector<std::string> ResourceManager::GetShadersName() const
	{
		std::vector<std::string> shaders_names;
		shaders_names.reserve(shader_registry_.size());
		for (auto& [name, _] : shader_registry_) {
			shaders_names.push_back(name);
		}
		return shaders_names;
	}

	void ResourceManager::BindShader(const string& name)
	{
		if (!rhi_device_) {
			return;
		}
		rhi_device_->BindShader(GetShaderDescByName(name));
	}

	bool ResourceManager::UseShader(const string& name)
	{
		const auto iter = shader_registry_.find(name);
		if (iter == shader_registry_.end()) {
			return false;
		}

		BindShader(name);
		if (rhi_device_ && rhi_device_->GetBackendType() == RhiBackendType::OpenGL) {
			if (!iter->second.opengl_shader) {
				return false;
			}
			iter->second.opengl_shader->use();
		}
		return true;
	}

	bool ResourceManager::ApplyShaderProperties(const string& name, const ShaderProperties& properties)
	{
		if (!UseShader(name)) {
			return false;
		}

		bool applied = false;
		for (const auto& property : properties) {
			applied = SetShaderProperty(name, property) || applied;
		}
		return applied || properties.empty();
	}

	bool ResourceManager::SetShaderProperty(const string& shader_name, const ShaderProperty& property)
	{
		auto iter = shader_registry_.find(shader_name);
		if (iter == shader_registry_.end()) {
			return false;
		}

		ShaderProperty stored_property = property;
		stored_property.name = NormalizeShaderPropertyName(stored_property.name);
		if (const auto* parameter_desc = FindParameterDesc(iter->second.material_desc, stored_property.name)) {
			stored_property.type = parameter_desc->type_name.empty() ? ToString(parameter_desc->type) : parameter_desc->type_name;
			stored_property.location = parameter_desc->binding;
		}
		else if (stored_property.type.empty()) {
			stored_property.type = GetShaderPropertyTypeName(stored_property.val);
		}

		iter->second.parameter_cache[stored_property.name] = stored_property;
		if (iter->second.opengl_shader) {
			return ApplyOpenGLShaderProperty(*iter->second.opengl_shader, stored_property);
		}
		return true;
	}

	bool ResourceManager::SetShaderProperty(const string& shader_name, std::string_view property_name, const ShaderProperty::Var& value)
	{
		ShaderProperty property;
		property.name = NormalizeShaderPropertyName(property_name);
		property.location = -1;
		property.val = value;
		return SetShaderProperty(shader_name, property);
	}

	RhiBackendType ResourceManager::GetActiveBackendType() const
	{
		return rhi_device_ ? rhi_device_->GetBackendType() : RhiBackendType::OpenGL;
	}

	void ResourceManager::RegisteredUIHandle(const string& name, QOpenGLWidget* ui_handle)
	{
		DeregisteredUIHandle(name);

		draw_ui_map_.emplace(name, ui_handle);

		QTimer* timer = new QTimer;
		QObject::connect(timer, &QTimer::timeout, [ui_handle]() {ui_handle->update(); });
		timer->start(5);
		ui_update_timer_map_.emplace(name, unique_ptr<QTimer>(timer));
	}

	void ResourceManager::DeregisteredUIHandle(const string& name)
	{		
		auto iter = draw_ui_map_.find(name);
		if (iter != draw_ui_map_.end()) {
			draw_ui_map_.erase(name);
			ui_update_timer_map_[name]->stop();
			ui_update_timer_map_.erase(name);
		}
	}

	QOpenGLWidget* ResourceManager::GetUISurfaceByName(const string& name)
	{
		auto iter = draw_ui_map_.find(name);
		if (iter != draw_ui_map_.end()) {
			return iter->second;
		}
		return nullptr;
	}

	void ResourceManager::RegisteredTexture(const TextureMsg& msg)
	{
		texture_require_upload_.push_back(msg);
	}

	void ResourceManager::RegisteredCubemap(const CubemapMsg& msg)
	{
		cubemap_require_upload_.push_back(msg);
	}

	void ResourceManager::DeregisteredTexture(const string& name)
	{		
		auto iter = texture_map_.find(name);
		if (iter != texture_map_.end()) {
			if (rhi_device_) {
				rhi_device_->DestroyTexture(RhiTextureHandle{ iter->second.id });
			}
			texture_map_.erase(iter);
		}
	}

	void ResourceManager::RegisteredTexture(const std::string& name, unsigned int tex)
	{
		DeregisteredTexture(name);
		texture_map_[name] = Texture{tex};
	}

	Texture ResourceManager::GetTextureByName(const string& name)
	{
		auto iter = texture_map_.find(name);
		if (iter != texture_map_.end()) {
			return iter->second;
		}
		return Texture{0};
	}

	void ResourceManager::SetRhiDevice(const shared_ptr<IRhiDevice>& rhi_device)
	{
		rhi_device_ = rhi_device;

		for (auto& mesh_not_set : mesh_require_upload_) {
			mesh_map_[mesh_not_set]->SetRhiDevice(rhi_device_);
		}
		mesh_require_upload_.clear();

		std::list<std::string> failed_link_shader;
		for (auto& shader_not_set : shader_require_upload_) {
			auto shader_desc_iter = shader_registry_.find(shader_not_set);
			if (shader_desc_iter == shader_registry_.end()) {
				continue;
			}

			if (rhi_device_ && rhi_device_->GetBackendType() == RhiBackendType::OpenGL) {
				shader_desc_iter->second.opengl_shader = std::make_unique<MyShader>(nullptr, shader_desc_iter->second.shader_desc);
				shader_desc_iter->second.opengl_shader->SetName(shader_not_set);
				shader_desc_iter->second.opengl_shader->SetRhiDevice(rhi_device_);
				if (!shader_desc_iter->second.opengl_shader->isLinked()) {
					std::cout << shader_not_set + " shader link failed\n";
					failed_link_shader.push_back(shader_not_set);
				}
				else {
					shader_desc_iter->second.material_desc = shader_desc_iter->second.opengl_shader->GetMaterialDesc();
					emit ShaderRegistered(shader_not_set);
				}
			}
			else {
				emit ShaderRegistered(shader_not_set);
			}
		}
		for (auto& shader_failed : failed_link_shader) {
			auto failed_iter = shader_registry_.find(shader_failed);
			if (failed_iter != shader_registry_.end()) {
				failed_iter->second.opengl_shader.reset();
			}
		}
		shader_require_upload_.clear();

		for (auto& [name, path, type, handle] : texture_require_upload_) {
			Texture texture;
			texture.id = static_cast<unsigned>(rhi_device_->LoadTexture2D(path).value);
			texture.type = type;
			if (handle) *handle = texture.id;
			if (texture.id) {
				texture_map_.emplace(name, texture);
			}
		}
		texture_require_upload_.clear();

		for (auto& [name, paths, type, handle] : cubemap_require_upload_) {
			Texture cubemap_texture;
			cubemap_texture.id = static_cast<unsigned>(rhi_device_->LoadCubemap(paths).value);
			cubemap_texture.type = type;
			if (handle) *handle = cubemap_texture.id;
			if (cubemap_texture.id) {
				texture_map_.emplace(name, cubemap_texture);
			}
		}
		cubemap_require_upload_.clear();
	}

	void ResourceManager::tick(const shared_ptr<IRhiDevice>& rhi_device)
	{
		SetRhiDevice(rhi_device);
	}
}
