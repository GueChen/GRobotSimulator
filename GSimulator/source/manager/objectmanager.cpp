#include "manager/objectmanager.h"

#include "manager/modelmanager.h"
#include "manager/resourcemanager.h"
#include "function/adapter/modelloader_qgladapter.h"

#include "component/material_component.h"

// delete all using a better method as replace
#include "simplexmesh/gball.h"
//

GComponent::ObjectManager& GComponent::ObjectManager::getInstance() {
	static ObjectManager instance;
	return instance;
}

void GComponent::ObjectManager::Initialize()
{
	RegisterObject("cube",		"cube",		 "./asset/objects/cube.obj");
	RegisterObject("sphere",	"sphere",	 "./asset/objects/sphere.obj");
	RegisterObject("capsule",	"capsule",	 "./asset/objects/capsule.obj");
	RegisterObject("cylinder",	"cylinder",  "./asset/objects/cylinder.obj");
	RegisterObject("plane",		"floor",	 "./asset/objects/floor.obj");

	// FIX THIS: Use custom to replce all obj
	{				
		ResourceManager::getInstance().RegisteredMesh("sphere", new RenderMesh(GBall(glm::zero<glm::vec3>(), 0.5, glm::zero<glm::vec3>(), 20).GetBallMesh()));
	}
}

bool GComponent::ObjectManager::RegisterObject(const string& obj_name, const string& mesh_name, const string& mesh_asset_path)
{
	if (obj_lists_count_table_.count(obj_name)) return false;
	obj_lists_count_table_.emplace(obj_name, 0);
	obj_properties_table_.emplace(obj_name, mesh_name);

	if (!mesh_asset_path.empty())
	{
		ResourceManager::getInstance().RegisteredMesh(mesh_name, QGL::ModelLoader::getMeshPtr(mesh_asset_path));
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

bool GComponent::ObjectManager::CreateInstanceWithModelMat(const string& obj_name, const Mat4& model_mat)
{
	auto iter = obj_lists_count_table_.find(obj_name);
	if (iter == obj_lists_count_table_.end()) return false;
	string name = obj_name + std::to_string(obj_lists_count_table_[obj_name]);
	auto&  mesh = obj_properties_table_[obj_name];
	Model* model= new Model(name, mesh, model_mat);
	model->RegisterComponent(std::make_unique<MaterialComponent>(model, "pbr", true));
	ModelManager::getInstance().RegisteredModel(name, std::move(model));	
	++obj_lists_count_table_[obj_name];
	return true;
}

void GComponent::ObjectManager::CreateInstance(const string& obj_name)
{
	CreateInstanceWithModelMat(obj_name, Mat4::Identity());
}
