#include "model/robot/aubo_i3_model.h"

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

namespace GComponent {

static bool aubo_resouce_init = false;

AUBO_I3_MODEL::AUBO_I3_MODEL(Model* parent_ptr, Mat4 transform):
	Model(parent_ptr)
{
	GetTransform()->SetModelLocal(transform);
	InitializeMeshResource();
	InitializeModelResource();
}

void AUBO_I3_MODEL::tickImpl(float delta_time)
{
	
}

void AUBO_I3_MODEL::InitializeModelResource()
{
	using Vec3 = Eigen::Vector3f;
	ModelManager& manager = ModelManager::getInstance();
	std::array<Model*, 7> models_tmp = { nullptr };
	std::array<Vec3, 7> local_trans = {
		//local transx        y         z           (.m)
		Vec3(0.0f,     0.0f,     0.0f),
		Vec3(0.0f,     0.0f,     0.098f),
		Vec3(0.0f,     0.0538f,  0.07308f),
		Vec3(0.0f,     0.0087f,  0.26584f),
		Vec3(0.0f,     0.0083f,  0.25658f),
		Vec3(0.05417f, 0.048f,   0.0f),
		Vec3(0.048f,   0.0f,	 0.06037f)
	};

	manager.RegisteredModel("aubo_i3_robot", this);
	for (int i = 0; i < 7; ++i) {
		string name = "aubo_i3_link_" + std::to_string(i);
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
		Vec3::UnitY(),
		Vec3::UnitY(),
		Vec3::UnitX(),
		Vec3::UnitZ()
	};

	std::vector<JointComponent*> joints;
	for (int i = 1; i < 7; ++i) {
		Component* com = models_tmp[i]->RegisterComponent(std::make_unique<JointComponent>(models_tmp[i], axis_binds[i - 1]));
		joints.push_back(static_cast<JointComponent*>(com));
	}

	SE3f end_trans_mat = SE3f::Identity();
	end_trans_mat.block(0, 3, 3, 1) = Vec3(0.10, 0.12, 0.80);

	RegisterComponent(std::make_unique<JointGroupComponent>(this, joints));
	RegisterComponent(std::make_unique<KinematicComponent>(end_trans_mat, this));
	RegisterComponent(std::make_unique<TrackerComponent>(this));

}

void AUBO_I3_MODEL::InitializeMeshResource()
{
	if (!aubo_resouce_init) {
		ResourceManager& scene_manager = ResourceManager::getInstance();				
#define AuboPair(name)\
	#name, QGL::ModelLoader::getMeshPtr("./asset/stls/aubo_i3/"#name".STL")
		scene_manager.RegisteredMesh(AuboPair(aubo_i3_base));
		scene_manager.RegisteredMesh(AuboPair(aubo_i3_link_0));
		scene_manager.RegisteredMesh(AuboPair(aubo_i3_link_1));
		scene_manager.RegisteredMesh(AuboPair(aubo_i3_link_2));
		scene_manager.RegisteredMesh(AuboPair(aubo_i3_link_3));
		scene_manager.RegisteredMesh(AuboPair(aubo_i3_link_4));
		scene_manager.RegisteredMesh(AuboPair(aubo_i3_link_5));
		scene_manager.RegisteredMesh(AuboPair(aubo_i3_link_6));
#undef AuboPair

		aubo_resouce_init = true;
	}
}

}