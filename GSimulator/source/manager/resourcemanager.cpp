#include "manager/resourcemanager.h"

#include <QtCore/QMetaType>
#include <iostream>
#include <format>

namespace GComponent {
	using std::move;

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
		shader_desc_map_.emplace(shader_desc.name, shader_desc);
		material_desc_map_.emplace(shader_desc.name, RhiMaterialDesc{
			.shader_name = shader_desc.name,
			.backend = shader_desc.backend
		});
	}

	void ResourceManager::RegisteredShader(const string& name, MyShader* raw_ptr_shader)
	{
		if (!raw_ptr_shader) {
			std::cerr << "RegisteredShader failed: shader is nullptr, name = " << name << '\n';
			return;
		}
		DeregisteredSpecificMapElement(shader_map_, name);
		shader_require_upload_.push_back(name);
		raw_ptr_shader->SetName(name);
		shader_map_.emplace(name, move(unique_ptr<MyShader>(raw_ptr_shader)));
	}

	void ResourceManager::DeregisteredShader(const string& name)
	{
		DeregisteredSpecificMapElement(shader_map_, name);
		DeregisteredSpecificMapElement(shader_desc_map_, name);
		DeregisteredSpecificMapElement(material_desc_map_, name);
	}

	MyShader* ResourceManager::GetShaderByName(const string& name)
	{
		auto iter = shader_map_.find(name);
		if (iter != shader_map_.end()) {
			return iter->second.get();
		}
		return nullptr;
	}

	const RhiShaderDesc* ResourceManager::GetShaderDescByName(const string& name) const
	{
		auto iter = shader_desc_map_.find(name);
		if (iter != shader_desc_map_.end()) {
			return &iter->second;
		}
		return nullptr;
	}

	const RhiMaterialDesc* ResourceManager::GetMaterialDescByShaderName(const string& name) const
	{
		auto iter = material_desc_map_.find(name);
		if (iter != material_desc_map_.end()) {
			return &iter->second;
		}
		return nullptr;
	}

	std::vector<std::string> ResourceManager::GetShadersName() const
	{
		std::vector<std::string> shaders_names;
		shaders_names.reserve(shader_desc_map_.size());
		for (auto& [name, _] : shader_desc_map_) {
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
			const auto shader_desc_iter = shader_desc_map_.find(shader_not_set);
			if (shader_desc_iter == shader_desc_map_.end()) {
				continue;
			}

			if (rhi_device_ && rhi_device_->GetBackendType() == RhiBackendType::OpenGL) {
				DeregisteredSpecificMapElement(shader_map_, shader_not_set);
				shader_map_.emplace(shader_not_set, std::make_unique<MyShader>(nullptr, shader_desc_iter->second));
				shader_map_[shader_not_set]->SetName(shader_not_set);
				shader_map_[shader_not_set]->SetRhiDevice(rhi_device_);
				if (!shader_map_[shader_not_set]->isLinked()) {
					std::cout << shader_not_set + " shader link failed\n";
					failed_link_shader.push_back(shader_not_set);
				}
				else {
					material_desc_map_[shader_not_set] = shader_map_[shader_not_set]->GetMaterialDesc();
					emit ShaderRegistered(shader_not_set);
				}
			}
			else {
				emit ShaderRegistered(shader_not_set);
			}
		}
		for (auto& shader_failed : failed_link_shader) {
			shader_map_.erase(shader_failed);
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
