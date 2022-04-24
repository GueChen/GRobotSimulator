#include "scenemanager.h"

#include "render/mesh.h"
#include "render/myshader.h"

namespace GComponent {
	using std::move;

	void SceneManager::RegisteredMesh(const string& name, Mesh* raw_ptr_mesh) {
		mesh_map_.emplace(name, unique_ptr<Mesh>(raw_ptr_mesh));
	}

	void SceneManager::DeregisteredMesh(const string& name)
	{
		auto iter = mesh_map_.find(name);
		if (iter != mesh_map_.end()) {
			mesh_map_.erase(name);
		}
	}

	Mesh* SceneManager::GetMeshByName(const string& name) {
		auto iter = mesh_map_.find(name);
		if (iter != mesh_map_.end()) {
			return iter->second.get();
		}
		return nullptr;
	}

	void SceneManager::RegisteredShader(const string& name, MyShader* raw_ptr_shader)
	{
		shader_map_.emplace(name, unique_ptr<MyShader>(raw_ptr_shader));
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
}
