#ifndef ROBOT_MODEL_BUILDER_H
#define ROBOT_MODEL_BUILDER_H

#include "model/robot/robot_asset.h"

namespace GComponent {

class JointComponent;

struct BuiltRobotModel {
	std::vector<Model*> links;
	std::vector<JointComponent*> joints;
};

class RobotModelBuilder {
public:
	RobotModelBuilder() = delete;

	static void RegisterMeshes(const RobotAsset& asset);
	static BuiltRobotModel Build(Model& root, const RobotAsset& asset, const std::string& instance_suffix = "");
	static void ApplyMaterialPreset(Model& root, const RobotMaterialPreset& preset);
};

} // namespace GComponent

#endif // ROBOT_MODEL_BUILDER_H
