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
		shader_map_.emplace("picking", std::make_unique<MyShader>(nullptr, PathVert(picking), PathFrag(picking)));
	}

	void ResourceManager::RegisteredMesh(const string& name, RenderMesh* raw_ptr_mesh) 
	{
		DeregisteredMesh(name);
		mesh_require_gl_.push_back(name);
		mesh_map_.emplace(name, unique_ptr<RenderMesh>(raw_ptr_mesh));
	}

	RenderMesh* ResourceManager::GetMeshByName(const string& name) {
		auto iter = mesh_map_.find(name);
		if (iter != mesh_map_.end()) {
			return iter->second.get();
		}
		return nullptr;
	}

	void ResourceManager::RegisteredShader(const string& name, MyShader* raw_ptr_shader)
	{
		DeregisteredShader(name);
		shader_require_gl_.push_back(name);
		raw_ptr_shader->SetName(name);
		shader_map_.emplace(name, move(unique_ptr<MyShader>(raw_ptr_shader)));
	}

	MyShader* ResourceManager::GetShaderByName(const string& name)
	{
		auto iter = shader_map_.find(name);
		if (iter != shader_map_.end()) {
			return iter->second.get();
		}
		return nullptr;
	}

	std::vector<std::string> ResourceManager::GetShadersName() const
	{
		std::vector<std::string> shaders_names;
		shaders_names.reserve(shader_map_.size());
		for (auto& [name, _] : shader_map_) {
			shaders_names.push_back(name);
		}
		return shaders_names;
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
		texture_require_gl_.push_back(msg);
	}

	void ResourceManager::RegisteredCubemap(const CubemapMsg& msg)
	{
		cubemap_require_gl_.push_back(msg);
	}

	void ResourceManager::DeregisteredTexture(const string& name)
	{		
		auto iter = texture_map_.find(name);
		if (iter != texture_map_.end()) {
			gl_->glDeleteTextures(1, &iter->second.id);
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

	void ResourceManager::SetGL(const shared_ptr<MyGL>& gl)
	{
		gl_ = gl;
		// setting gl context to mesh resource
		for (auto& mesh_not_set : mesh_require_gl_) {
			mesh_map_[mesh_not_set]->SetGL(gl);
		}
		mesh_require_gl_.clear();

		std::list<std::string> failed_link_shader;
		for (auto& shader_not_set : shader_require_gl_) {
			shader_map_[shader_not_set]->SetGL(gl);
			if (!shader_map_[shader_not_set]->isLinked()) {
				std::cout << shader_not_set + " shader link failed\n";
				failed_link_shader.push_back(shader_not_set);
			}
			else {
				emit ShaderRegistered(shader_not_set);
			}
		}
		// cleaning the failed register shader
		for (auto& shader_failed : failed_link_shader) {
			shader_map_.erase(shader_failed);
		}
		shader_require_gl_.clear();

		// register gl contex for texture
		for (auto& [name, path, type, handle] : texture_require_gl_) {
			Texture texture;
			texture.id = gl_->LoadTexture(path.data());
			texture.type = type;
			if (handle) *handle = texture.id;
			if (texture.id) {
				texture_map_.emplace(name, texture);
			}
		}
		texture_require_gl_.clear();

		// register gl contex for cubemap
		for (auto& [name, paths, type, handle] : cubemap_require_gl_) {
			Texture cubemap_texture;
			cubemap_texture.id = gl_->LoadCubemap(paths);
			cubemap_texture.type = type;
			if (handle) *handle = cubemap_texture.id;
			if (cubemap_texture.id) {
				texture_map_.emplace(name, cubemap_texture);
			}
		}
		cubemap_require_gl_.clear();
	}

	void ResourceManager::tick(const shared_ptr<MyGL>& gl)
	{
		SetGL(gl);
	}
}
