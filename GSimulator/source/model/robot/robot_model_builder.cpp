#include "model/robot/robot_model_builder.h"

#include "component/joint_component.h"
#include "component/joint_group_component.h"
#include "component/kinematic_component.h"
#include "component/material_component.h"
#include "component/rigidbody_component.h"
#include "component/tracker_component.h"
#include "function/adapter/modelloader_qgladapter.h"
#include "manager/modelmanager.h"
#include "manager/resourcemanager.h"

#include <memory>
#include <stack>

namespace GComponent {

namespace {

void RegisterMeshIfNeeded(const RobotMeshDesc& mesh_desc)
{
	if (mesh_desc.mesh_name.empty() || mesh_desc.mesh_path.empty()) {
		return;
	}

	ResourceManager& resource_manager = ResourceManager::getInstance();
	if (resource_manager.GetMeshByName(mesh_desc.mesh_name)) {
		return;
	}

	resource_manager.RegisteredMesh(mesh_desc.mesh_name, QGL::ModelLoader::getMeshPtr(mesh_desc.mesh_path));
}

void RegisterCollider(Model& model, const RobotColliderDesc& collider)
{
	CollisionGroup group{ 0, 0, 0, 0 };
	switch (collider.shape) {
	case RobotColliderShape::Box:
		model.RegisterComponent(std::make_unique<RigidbodyComponent>(
			&model,
			collider.local_transform,
			collider.extents.x(),
			collider.extents.y(),
			collider.extents.z(),
			group));
		break;
	case RobotColliderShape::Capsule:
		model.RegisterComponent(std::make_unique<RigidbodyComponent>(
			&model,
			collider.local_transform,
			collider.extents.x(),
			collider.extents.y(),
			group));
		break;
	case RobotColliderShape::Sphere:
		model.RegisterComponent(std::make_unique<RigidbodyComponent>(
			&model,
			collider.local_transform,
			collider.extents.x(),
			group));
		break;
	}
}

void ApplyMaterialProperties(MaterialComponent& material, const RobotMaterialPreset& preset)
{
	if (material.GetProperties().empty()) {
		material.SetShader(material.GetShader());
	}

	auto& props = material.GetProperties();
	for (auto& [_, name, __, val] : props) {
		if (name == "accept shadow" && preset.accept_shadow.has_value()) {
			val = *preset.accept_shadow;
		}
		else if (name == "albedo color" && preset.albedo_color.has_value()) {
			val = *preset.albedo_color;
		}
		else if (name == "ao" && preset.ao.has_value()) {
			val = *preset.ao;
		}
		else if (name == "metallic" && preset.metallic.has_value()) {
			val = *preset.metallic;
		}
		else if (name == "roughness" && preset.roughness.has_value()) {
			val = *preset.roughness;
		}
	}
}

} // namespace

void RobotModelBuilder::RegisterMeshes(const RobotAsset& asset)
{
	for (const auto& mesh : asset.extra_meshes) {
		RegisterMeshIfNeeded(mesh);
	}

	for (const auto& link : asset.links) {
		RegisterMeshIfNeeded(RobotMeshDesc{ link.mesh_name, link.mesh_path });
	}
}

BuiltRobotModel RobotModelBuilder::Build(Model& root, const RobotAsset& asset, const std::string& instance_suffix)
{
	RegisterMeshes(asset);

	BuiltRobotModel result;
	result.links.resize(asset.links.size(), nullptr);

	ModelManager& model_manager = ModelManager::getInstance();
	model_manager.RegisteredModel(asset.root_name + instance_suffix, &root);

	for (size_t i = 0; i < asset.links.size(); ++i) {
		const RobotLinkDesc& link = asset.links[i];
		Model* parent = &root;
		if (link.parent_index >= 0 && static_cast<size_t>(link.parent_index) < result.links.size()) {
			parent = result.links[link.parent_index];
		}

		Model* link_model = new Model(
			link.model_name + instance_suffix,
			link.mesh_name,
			link.local_translation,
			link.local_rotation,
			link.local_scale,
			parent);
		link_model->RegisterComponent(std::make_unique<MaterialComponent>(link_model, link.shader, link.cast_shadow));
		model_manager.RegisteredModel(link_model->getName(), link_model);
		result.links[i] = link_model;

		for (const auto& collider : link.colliders) {
			RegisterCollider(*link_model, collider);
		}
	}

	for (const RobotJointDesc& joint_desc : asset.joints) {
		if (joint_desc.link_index < 0 || static_cast<size_t>(joint_desc.link_index) >= result.links.size()) {
			continue;
		}

		Model* link_model = result.links[joint_desc.link_index];
		Component* component = link_model->RegisterComponent(std::make_unique<JointComponent>(link_model, joint_desc.axis));
		JointComponent* joint = static_cast<JointComponent*>(component);
		if (joint_desc.position_limit.has_value()) {
			joint->SetPosLimit(joint_desc.position_limit->first, joint_desc.position_limit->second);
		}
		result.joints.push_back(joint);
	}

	root.RegisterComponent(std::make_unique<JointGroupComponent>(&root, result.joints));
	root.RegisterComponent(std::make_unique<KinematicComponent>(asset.end_effector_transform, &root));
	root.RegisterComponent(std::make_unique<TrackerComponent>(&root));
	return result;
}

void RobotModelBuilder::ApplyMaterialPreset(Model& root, const RobotMaterialPreset& preset)
{
	std::stack<Model*> stack;
	stack.push(&root);
	while (!stack.empty()) {
		Model* current = stack.top();
		stack.pop();
		if (MaterialComponent* material = current->GetComponent<MaterialComponent>()) {
			ApplyMaterialProperties(*material, preset);
		}

		for (Model* child : current->getChildren()) {
			stack.push(child);
		}
	}
}

} // namespace GComponent
