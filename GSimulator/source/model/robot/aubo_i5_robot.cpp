#include "model/robot/aubo_i5_model.h"


#include "manager/modelmanager.h"
#include "manager/resourcemanager.h"
#include "manager/rendermanager.h"

#include "render/rendermesh.h"

#include "component/transform_component.h"
#include "component/joint_component.h"
#include "component/joint_group_component.h"
#include "component/kinematic_component.h"
#include "component/tracker_component.h"
#include "component/rigidbody_component.h"
#include "component/material_component.h"

#include "function/adapter/modelloader_qgladapter.h"

#include <array>
#include <string>

namespace GComponent{

static bool aubo_i5_resouce_init = false;
AUBO_I5_MODEL::AUBO_I5_MODEL(Model* parent_ptr, Mat4 transform):
	Model(parent_ptr)
{
	GetTransform()->SetModelLocal(transform);
	InitializeMeshResource();
	InitializeModelResource();
}

void AUBO_I5_MODEL::InitializeModelResource()
{
	using Vec3 = Eigen::Vector3f;
	ModelManager& manager = ModelManager::getInstance();
	std::array<Model*, 7> models_tmp = { nullptr };
	std::array<Vec3, 7> local_trans = {
		//local transx        y         z           (.m)
		Vec3(0.0f,     0.0f,     0.0f),
		Vec3(0.0f,     0.0f,     0.04370f),
		Vec3(0.0f,     0.0622f,  0.07830f),
		Vec3(0.0f,     0.0068f,  0.40626f),
		Vec3(0.0f,     0.0045f,  0.3779f),
		Vec3(0.0f,	   0.04817f, 0.05433f),
		Vec3(0.0f,     0.05463f, 0.04823)
	};

	manager.RegisteredModel("aubo_i5_robot", this);
	for (int i = 0; i < 7; ++i) {
		string name = "aubo_i5_link_" + std::to_string(i);
		models_tmp[i] = new Model(name,
			name,
			local_trans[i],
			Vec3::Zero(),
			Vec3::Ones(),
			i > 0 ? models_tmp[i - 1] : this);
		models_tmp[i]->RegisterComponent(std::make_unique<MaterialComponent>(models_tmp[i], "pbr", true));
		manager.RegisteredModel(name, models_tmp[i]);
	}

	std::array<Vec3, 7> axis_binds = {
		Vec3::UnitZ(),
		Vec3::UnitY(),
		-Vec3::UnitY(),
		Vec3::UnitY(),
		Vec3::UnitZ(),
		Vec3::UnitY()
	};

	std::vector<JointComponent*> joints;
	for (int i = 1; i < 7; ++i) {
		Component* com = models_tmp[i]->RegisterComponent(std::make_unique<JointComponent>(models_tmp[i], axis_binds[i - 1]));
		joints.push_back(static_cast<JointComponent*>(com));
	}

	SE3f end_trans_mat = SE3f::Identity();
	end_trans_mat.block(0, 3, 3, 1) = Vec3(0.00, 0.21527, 1.00803);

	RegisterComponent(std::make_unique<JointGroupComponent>(this, joints));
	RegisterComponent(std::make_unique<KinematicComponent>(end_trans_mat, this));
	RegisterComponent(std::make_unique<TrackerComponent>(this));
}

void AUBO_I5_MODEL::InitializeMeshResource()
{
	if (!aubo_i5_resouce_init) {
		ResourceManager& scene_manager = ResourceManager::getInstance();
#define AuboPair(name)\
	#name, QGL::ModelLoader::getMeshPtr("./asset/stls/aubo_i5/"#name".STL")
		scene_manager.RegisteredMesh(AuboPair(aubo_i5_link_0));
		scene_manager.RegisteredMesh(AuboPair(aubo_i5_link_1));
		scene_manager.RegisteredMesh(AuboPair(aubo_i5_link_2));
		scene_manager.RegisteredMesh(AuboPair(aubo_i5_link_3));
		scene_manager.RegisteredMesh(AuboPair(aubo_i5_link_4));
		scene_manager.RegisteredMesh(AuboPair(aubo_i5_link_5));
		scene_manager.RegisteredMesh(AuboPair(aubo_i5_link_6));
#undef AuboPair

		aubo_i5_resouce_init = true;
	}
}

}