#include "model/robot/robot_asset.h"

#include "function/adapter/modelloader_qgladapter.h"

#include <GComponent/GNumerical.hpp>

#include <Eigen/Geometry>

#include <array>

namespace GComponent {

namespace {

Mat4 Translate(const Vec3& translation)
{
	Eigen::Affine3f transform = Eigen::Affine3f::Identity();
	transform.translate(translation);
	return transform.matrix();
}

Mat4 TranslateRotateY90(const Vec3& translation)
{
	Eigen::Affine3f transform = Eigen::Affine3f::Identity();
	transform.translate(translation).rotate(Eigen::AngleAxisf(EIGEN_PI * 0.5f, Vec3::UnitY()));
	return transform.matrix();
}

Mat4 EndTransform(const Vec3& translation)
{
	Mat4 transform = Mat4::Identity();
	transform.block(0, 3, 3, 1) = translation;
	return transform;
}

Mat4 PlatformArmTransform(const Vec3& translation, float x_degrees, float z_degrees)
{
	Eigen::Affine3f transform = Eigen::Affine3f::Identity();
	transform.translate(translation);
	transform.rotate(Eigen::AngleAxisf(DegreeToRadius(x_degrees), Vec3::UnitX()));
	transform.rotate(Eigen::AngleAxisf(DegreeToRadius(z_degrees), Vec3::UnitZ()));
	return transform.matrix();
}

} // namespace

const RobotAsset& GetKukaIiwaAsset()
{
	static const RobotAsset asset = [] {
		RobotAsset data;
		data.root_name = "kuka_iiwa_robot";

		constexpr float kLimitTolerance = 3.5f;
		const std::array<Vec3, 8> local_trans = {
			Vec3(0.0f, 0.0f, 0.0f),
			Vec3(0.0f, 0.0f, 0.1575f),
			Vec3(0.0f, 0.0f, 0.2025f),
			Vec3(0.0f, 0.0f, 0.2045f),
			Vec3(0.0f, 0.0f, 0.2155f),
			Vec3(0.0f, 0.0f, 0.1845f),
			Vec3(0.0f, -0.0607f, 0.2155f),
			Vec3(0.0f, 0.0607f, 0.0809f)
		};

		data.links = {
			RobotLinkDesc{ "kuka_iiwa_robot_link_0", "kuka_iiwa_robot_link_0", cPathModel("binary/iiwa14_base_binary.STL"), -1, local_trans[0] },
			RobotLinkDesc{ "kuka_iiwa_robot_link_1", "kuka_iiwa_robot_link_1", cPathModel("binary/iiwa14_link_1_binary.STL"), 0, local_trans[1] },
			RobotLinkDesc{ "kuka_iiwa_robot_link_2", "kuka_iiwa_robot_link_2", cPathModel("binary/iiwa14_link_2_binary.STL"), 1, local_trans[2] },
			RobotLinkDesc{ "kuka_iiwa_robot_link_3", "kuka_iiwa_robot_link_3", cPathModel("binary/iiwa14_link_3_binary.STL"), 2, local_trans[3] },
			RobotLinkDesc{ "kuka_iiwa_robot_link_4", "kuka_iiwa_robot_link_4", cPathModel("binary/iiwa14_link_4_binary.STL"), 3, local_trans[4] },
			RobotLinkDesc{ "kuka_iiwa_robot_link_5", "kuka_iiwa_robot_link_5", cPathModel("binary/iiwa14_link_5_binary.STL"), 4, local_trans[5] },
			RobotLinkDesc{ "kuka_iiwa_robot_link_6", "kuka_iiwa_robot_link_6", cPathModel("binary/iiwa14_link_6_binary.STL"), 5, local_trans[6] },
			RobotLinkDesc{ "kuka_iiwa_robot_link_7", "kuka_iiwa_robot_link_7", cPathModel("binary/flansch_extern_binary.STL"), 6, local_trans[7] }
		};

		const Vec3 base_collider_center = 0.5f * local_trans[1];
		const float base_collider_half = base_collider_center.norm();
		data.links[0].colliders.push_back(RobotColliderDesc{
			.shape = RobotColliderShape::Box,
			.local_transform = Translate(base_collider_center),
			.extents = Vec3(base_collider_half, base_collider_half, base_collider_half)
		});

		const Vec3 link1_collider_center = 0.5f * local_trans[2];
		data.links[1].colliders.push_back(RobotColliderDesc{
			.shape = RobotColliderShape::Capsule,
			.local_transform = TranslateRotateY90(link1_collider_center),
			.extents = Vec3(0.12f, link1_collider_center.norm() - 0.07f, 0.0f)
		});

		const Vec3 link7_z_only = Vec3(0.0f, 0.0f, local_trans[7].z());
		data.links[6].colliders.push_back(RobotColliderDesc{
			.shape = RobotColliderShape::Capsule,
			.local_transform = TranslateRotateY90(0.45f * local_trans[7]),
			.extents = Vec3(0.14f, 0.35f * link7_z_only.norm(), 0.0f)
		});

		const std::array<Vec3, 7> axes = {
			Vec3::UnitZ(), Vec3::UnitY(), Vec3::UnitZ(), -Vec3::UnitY(),
			Vec3::UnitZ(), Vec3::UnitY(), Vec3::UnitZ()
		};
		const std::array<std::pair<float, float>, 7> limits = {
			std::pair{ DegreeToRadius(-170.0f + kLimitTolerance), DegreeToRadius(180.0f - kLimitTolerance) },
			std::pair{ DegreeToRadius(-120.0f + kLimitTolerance), DegreeToRadius(120.0f - kLimitTolerance) },
			std::pair{ DegreeToRadius(-170.0f + kLimitTolerance), DegreeToRadius(170.0f - kLimitTolerance) },
			std::pair{ DegreeToRadius(-120.0f + kLimitTolerance), DegreeToRadius(120.0f - kLimitTolerance) },
			std::pair{ DegreeToRadius(-170.0f + kLimitTolerance), DegreeToRadius(170.0f - kLimitTolerance) },
			std::pair{ DegreeToRadius(-120.0f + kLimitTolerance), DegreeToRadius(120.0f - kLimitTolerance) },
			std::pair{ DegreeToRadius(-175.0f + kLimitTolerance), DegreeToRadius(175.0f - kLimitTolerance) }
		};
		for (int i = 0; i < 7; ++i) {
			data.joints.push_back(RobotJointDesc{
				.link_index = i + 1,
				.axis = axes[i],
				.position_limit = limits[i]
			});
		}

		data.end_effector_transform = EndTransform(Vec3(0.0f, 0.0f, 1.332f));
		return data;
	}();
	return asset;
}

const RobotAsset& GetAuboI3Asset()
{
	static const RobotAsset asset = [] {
		RobotAsset data;
		data.root_name = "aubo_i3_robot";
		data.extra_meshes.push_back(RobotMeshDesc{ "aubo_i3_base", "./asset/stls/aubo_i3/aubo_i3_base.STL" });

		const std::array<Vec3, 7> local_trans = {
			Vec3(0.0f, 0.0f, 0.0f),
			Vec3(0.0f, 0.0f, 0.098f),
			Vec3(0.0f, 0.0538f, 0.07308f),
			Vec3(0.0f, 0.0087f, 0.26584f),
			Vec3(0.0f, 0.0083f, 0.25658f),
			Vec3(0.05417f, 0.048f, 0.0f),
			Vec3(0.048f, 0.0f, 0.06037f)
		};

		for (int i = 0; i < 7; ++i) {
			const std::string name = "aubo_i3_link_" + std::to_string(i);
			data.links.push_back(RobotLinkDesc{
				.model_name = name,
				.mesh_name = name,
				.mesh_path = "./asset/stls/aubo_i3/" + name + ".STL",
				.parent_index = i == 0 ? -1 : i - 1,
				.local_translation = local_trans[i]
			});
		}

		const std::array<Vec3, 6> axes = {
			Vec3::UnitZ(), Vec3::UnitY(), Vec3::UnitY(),
			Vec3::UnitY(), Vec3::UnitX(), Vec3::UnitZ()
		};
		for (int i = 0; i < 6; ++i) {
			data.joints.push_back(RobotJointDesc{
				.link_index = i + 1,
				.axis = axes[i]
			});
		}

		data.end_effector_transform = EndTransform(Vec3(0.10f, 0.12f, 0.80f));
		return data;
	}();
	return asset;
}

const DualArmPlatformAsset& GetDualArmPlatformAsset()
{
	static const DualArmPlatformAsset asset = {
		.body_mesh = RobotMeshDesc{ "dual_arm_body", cPathModel("binary/platform_binary.STL") },
		.body_material = RobotMaterialPreset{
			.accept_shadow = true,
			.ao = 0.05f,
			.metallic = 1.0f,
			.roughness = 0.25f
		},
		.left_arm = PlatformRobotInstanceDesc{
			.local_transform = PlatformArmTransform(Vec3(0.0f, 0.193f, 1.217f), -30.0f, 45.0f),
			.material = RobotMaterialPreset{
				.accept_shadow = true,
				.albedo_color = glm::vec3(0.80f, 0.05f, 0.0f),
				.ao = 0.05f,
				.metallic = 0.98f,
				.roughness = 0.25f
			}
		},
		.right_arm = PlatformRobotInstanceDesc{
			.local_transform = PlatformArmTransform(Vec3(0.0f, -0.193f, 1.217f), 30.0f, -45.0f),
			.material = RobotMaterialPreset{
				.accept_shadow = true,
				.albedo_color = glm::vec3(1.0f),
				.ao = 0.05f,
				.metallic = 0.25f,
				.roughness = 0.25f
			}
		}
	};
	return asset;
}

} // namespace GComponent
