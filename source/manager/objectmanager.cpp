#include "manager/objectmanager.h"

#include "manager/modelmanager.h"
#include "manager/scenemanager.h"
#include "function/adapter/modelloader_qgladapter.h"
#include "render/myshader.h"

GComponent::ObjectManager& GComponent::ObjectManager::getInstance() {
	static ObjectManager instance;
	return instance;
}

void GComponent::ObjectManager::Initialize()
{
	RegisterObject("cube",	"cube", "color", "./asset/objects/cube.obj",			PathVert(Color), PathFrag(Color));
	RegisterObject("sphere", "sphere", "color", "./asset/objects/sphere.obj",		PathVert(Color), PathFrag(Color));
	RegisterObject("capsule", "capsule", "color", "./asset/objects/capsule.obj",	PathVert(Color), PathFrag(Color));
	RegisterObject("cylinder", "cylinder", "color", "./asset/objects/cylinder.obj",	PathVert(Color), PathFrag(Color));
	RegisterObject("plane", "floor", "color", "./asset/objects/floor.obj",			PathVert(Color), PathFrag(Color));
}

bool GComponent::ObjectManager::RegisterObject(const string& obj_name, const string& mesh_name, const string& shader_name, const string& mesh_asset_path, const string& shader_vert, const string& shader_frag)
{
	if (obj_lists_count_table_.count(obj_name)) return false;
	obj_lists_count_table_.emplace(obj_name, 0);
	obj_properties_table_.emplace(obj_name, std::make_pair(mesh_name, shader_name));

	if (!mesh_asset_path.empty())
	{
		SceneManager::getInstance().RegisteredMesh(mesh_name, QGL::ModelLoader::getMeshPtr(mesh_asset_path));
	}
	if (!shader_vert.empty() && !shader_frag.empty())
	{
		if (!SceneManager::getInstance().GetShaderByName(shader_name)) {
			SceneManager::getInstance().RegisteredShader(shader_name, new MyShader(nullptr, shader_vert, shader_frag));
		}
	}

	return true;
}

bool GComponent::ObjectManager::DerigisterObject(const string& obj_name)
{
	auto iter = obj_lists_count_table_.find(obj_name);
	if (iter == obj_lists_count_table_.end()) return false;
	obj_lists_count_table_.erase(obj_name);
	obj_properties_table_.erase(obj_name);
	return true;
}

bool GComponent::ObjectManager::CreateInstanceWithModelMat(const string& obj_name, const glm::mat4& model_mat)
{
	auto iter = obj_lists_count_table_.find(obj_name);
	if (iter == obj_lists_count_table_.end()) return false;
	string name = obj_name + std::to_string(obj_lists_count_table_[obj_name]);
	auto& [mesh, shader] = obj_properties_table_[obj_name];
	ModelManager::getInstance().RegisteredModel(name, new Model(name, mesh, shader, model_mat));
	++obj_lists_count_table_[obj_name];
	return true;
}

void GComponent::ObjectManager::CreateInstance(const string& obj_name)
{
	CreateInstanceWithModelMat(obj_name, glm::mat4(1.0f));
}
