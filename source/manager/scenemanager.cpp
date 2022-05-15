#include "scenemanager.h"

#include <QtCore/QMetaType>
#include <iostream>
#include <format>

namespace GComponent {
	using std::move;

	SceneManager::SceneManager() {

	}

	SceneManager::~SceneManager() = default;

	void SceneManager::EnablePickingMode()
	{
		shader_map_.emplace("picking", std::make_unique<MyShader>(this, PathVert(picking), PathFrag(picking)));
	}

	void SceneManager::RegisteredMesh(const string& name, MeshComponent* raw_ptr_mesh) 
	{
		DeregisteredMesh(name);
		mesh_require_gl_.push_back(name);
		mesh_map_.emplace(name, unique_ptr<MeshComponent>(raw_ptr_mesh));
	}

	void SceneManager::DeregisteredMesh(const string& name)
	{
		auto iter = mesh_map_.find(name);
		if (iter != mesh_map_.end()) {
			mesh_map_.erase(name);
		}
	}

	MeshComponent* SceneManager::GetMeshByName(const string& name) {
		auto iter = mesh_map_.find(name);
		if (iter != mesh_map_.end()) {
			return iter->second.get();
		}
		return nullptr;
	}

	void SceneManager::RegisteredShader(const string& name, MyShader* raw_ptr_shader)
	{
		DeregisteredShader(name);
		shader_require_gl_.push_back(name);
		shader_map_.emplace(name, move(unique_ptr<MyShader>(raw_ptr_shader)));
	}

	void SceneManager::DeregisteredShader(const string& name)
	{
		auto iter = shader_map_.find(name);
		if (iter != shader_map_.end()) {
			shader_map_.erase(name);
		}
	}
	MyShader* SceneManager::GetShaderByName(const string& name)
	{
		auto iter = shader_map_.find(name);
		if (iter != shader_map_.end()) {
			return iter->second.get();
		}
		return nullptr;
	}
	void SceneManager::RegisteredUIHandle(const string& name, QOpenGLWidget* ui_handle)
	{
		DeregisteredUIHandle(name);

		draw_ui_map_.emplace(name, ui_handle);

		QTimer* timer = new QTimer(ui_handle);
		connect(timer, &QTimer::timeout, [ui_handle]() {ui_handle->update(); });
		timer->start(5);
		ui_update_timer_map_.emplace(name, unique_ptr<QTimer>(timer));
	}

	void SceneManager::DeregisteredUIHandle(const string& name)
	{
		auto iter = draw_ui_map_.find(name);
		if (iter != draw_ui_map_.end()) {
			draw_ui_map_.erase(name);
			ui_update_timer_map_[name]->stop();
			ui_update_timer_map_.erase(name);
		}
	}

	QOpenGLWidget* SceneManager::GetUISurfaceByName(const string& name)
	{
		auto iter = draw_ui_map_.find(name);
		if (iter != draw_ui_map_.end()) {
			return iter->second;
		}
		return nullptr;
	}

	void SceneManager::SetGL(const shared_ptr<MyGL>& gl)
	{
		for (auto& [name, mesh] : mesh_map_) {
			mesh->setGL(gl);
		}
		for (auto& [name, shader] : shader_map_) {			
			shader->setGL(gl);			
		}
		mesh_require_gl_.clear();
		shader_require_gl_.clear();
	}
	void SceneManager::tick(const shared_ptr<MyGL>& gl)
	{
		for (auto& mesh_not_set : mesh_require_gl_) {
			mesh_map_[mesh_not_set]->setGL(gl);
		}
		mesh_require_gl_.clear();
		for (auto& shader_not_set : shader_require_gl_) {
			shader_map_[shader_not_set]->setGL(gl);
		}
		shader_require_gl_.clear();
	}
}
