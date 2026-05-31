#ifndef ROBOT_ASSET_H
#define ROBOT_ASSET_H

#include "model/model.h"

#include <glm/glm.hpp>

#include <optional>
#include <string>
#include <vector>

namespace GComponent {

struct RobotMeshDesc {
	std::string mesh_name;
	std::string mesh_path;
};

enum class RobotColliderShape {
	Box,
	Capsule,
	Sphere
};

struct RobotColliderDesc {
	RobotColliderShape shape = RobotColliderShape::Box;
	Mat4 local_transform = Mat4::Identity();
	Vec3 extents = Vec3::Zero();
};

struct RobotLinkDesc {
	std::string model_name;
	std::string mesh_name;
	std::string mesh_path;
	int parent_index = -1;
	Vec3 local_translation = Vec3::Zero();
	Vec3 local_rotation = Vec3::Zero();
	Vec3 local_scale = Vec3::Ones();
	std::string shader = "pbr";
	bool cast_shadow = true;
	std::vector<RobotColliderDesc> colliders;
};

struct RobotJointDesc {
	int link_index = -1;
	Vec3 axis = Vec3::Zero();
	std::optional<std::pair<float, float>> position_limit;
};

struct RobotAsset {
	std::string root_name;
	std::vector<RobotMeshDesc> extra_meshes;
	std::vector<RobotLinkDesc> links;
	std::vector<RobotJointDesc> joints;
	Mat4 end_effector_transform = Mat4::Identity();
};

struct RobotMaterialPreset {
	std::optional<bool> accept_shadow;
	std::optional<glm::vec3> albedo_color;
	std::optional<float> ao;
	std::optional<float> metallic;
	std::optional<float> roughness;
};

struct PlatformRobotInstanceDesc {
	Mat4 local_transform = Mat4::Identity();
	RobotMaterialPreset material;
};

struct DualArmPlatformAsset {
	RobotMeshDesc body_mesh;
	RobotMaterialPreset body_material;
	PlatformRobotInstanceDesc left_arm;
	PlatformRobotInstanceDesc right_arm;
};

const RobotAsset& GetKukaIiwaAsset();
const RobotAsset& GetAuboI3Asset();
const DualArmPlatformAsset& GetDualArmPlatformAsset();

} // namespace GComponent

#endif // ROBOT_ASSET_H
