#include "manager/objectmanager.h"

#include "manager/modelmanager.h"
#include "manager/resourcemanager.h"
#include "function/adapter/modelloader_qgladapter.h"

#include "component/material_component.h"
#include "component/collider_component.h"
// delete all using a better method as replace
#include "simplexmesh/gball.h"
//

static void SetPBRRandomProperties(GComponent::MaterialComponent* material) {
	using namespace GComponent;
	auto get_random = []()->float {
		return rand() % 10000 / 10000.0f;
	};	
	auto& properties = material->GetProperties();
	for (auto& prop : properties) {
		std::cout << prop.name << std::endl;
		if (prop.name == "albedo color") {
			prop.val = glm::vec3(get_random(), get_random(), get_random());
		}
		else if (prop.name == "ao") {
			prop.val = get_random();
		}
		else if (prop.name == "metallic") {
			prop.val = get_random();
		}
		else if (prop.name == "roughness") {
			prop.val = get_random();
		}
	}

}

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
	auto material = model->RegisterComponent(std::make_unique<MaterialComponent>(model, "pbr", true));
	SetPBRRandomProperties(dynamic_cast<MaterialComponent*>(material));

	model->RegisterComponent(std::make_unique<ColliderComponent>(model));	

	ModelManager::getInstance().RegisteredModel(name, std::move(model));	
	++obj_lists_count_table_[obj_name];
	return true;
}

void GComponent::ObjectManager::CreateInstance(const string& obj_name)
{
	CreateInstanceWithModelMat(obj_name, Mat4::Identity());
}
