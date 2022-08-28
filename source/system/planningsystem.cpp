#include "system/planningsystem.h"

#include "manager/planningmanager.h"
#include "manager/modelmanager.h"
#include "manager/rendermanager.h"

#include "component/kinematic_component.h"

#include "motion/GMotion"

#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG


namespace GComponent {

PlanningSystem& PlanningSystem::getInstance()
{
	static PlanningSystem instance;
	return instance;
}

PlanningSystem::~PlanningSystem() = default;

/*___________________________________PROTECTED METHODS___________________________________________________*/
std::string PlanningSystem::SimpleGetObjName(const QString& obj_name)
{
	if (obj_name == "Left") {
		return "kuka_iiwa_robot_0";
	}
	else if(obj_name == "Right") {
		return "kuka_iiwa_robot_1";
	}
	return "";
}

/*____________________________________PUBLIC SLOTS_______________________________________________________*/
void PlanningSystem::ResponsePTPMotionJSpace(const QString& obj_name, float max_vel, float max_acc, std::vector<float> target_joints)
{
	Model* robot = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_name));
	if (robot) {
		PTPMotion motion(target_joints);
		motion.SetMaxLinVel(max_vel).SetMaxLinAcc(max_acc);
		JTrajFunc func = motion(robot);
		PlanningManager::getInstance().RegisterPlanningTask(robot, func, motion.GetTotalTime(), 40);
	}	
}

//FIXME: THIS METHOD NOT GOOD FOR ISOLATION
// Because no class called PTPMotion descarte, therfore must inducting kinematic component
// Consider add a creator mehotd into PTPMotion for descarte target case
void PlanningSystem::ResponsePTPMotionCSpace(const QString& obj_name, float max_vel, float max_acc, std::vector<float> target_descarte)
{
	Model* robot = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_name));
	if (!robot)		return; // object not exist
	KinematicComponent* kine_sdk = robot->GetComponent<KinematicComponent>(KinematicComponent::type_name.data());		
	if (!kine_sdk)	return; // kinematic com not exist

	std::vector<float> target_joints;			
	Eigen::Matrix4f goal_mat = robot->getModelMatrixWithoutScale().inverse() * STLUtils::toMat4f(target_descarte);
	bool result				 = kine_sdk->InverseKinematic(target_joints, goal_mat, kine_sdk->GetJointsPos());
	
	if (!result) {			// no solve
#ifdef _DEBUG
		std::cout << "Solve Failed PTP CSpace, Please Set Another Target.\n";
#endif // _DEBUG		
		return;
	}

	std::transform(target_joints.begin(), target_joints.end(), target_joints.begin(), ToStandarAngle);
	PTPMotion motion(target_joints);
	motion.SetMaxLinVel(max_vel).SetMaxLinAcc(max_acc);
	JTrajFunc func = motion(robot);
	PlanningManager::getInstance().RegisterPlanningTask(robot, func, motion.GetTotalTime(), 40);
	
}

void PlanningSystem::ResponseLineMotion(const QString& obj_name, float max_vel, float max_acc, float max_ang_vel, float max_ang_acc, std::vector<float> target_descarte)
{
	Model* robot = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_name));
	if (!robot)		return; // object not exist
		
	Eigen::Matrix4f goal_mat = STLUtils::toMat4f(target_descarte);
	LineMotion motion(goal_mat);
	motion.SetMaxLinVel(max_vel).SetMaxLinAcc(max_acc).
		   SetMaxAngVel(max_ang_vel).SetMaxAngAcc(max_ang_acc);

	Trajectory func = motion(robot);

	Vec3 ini  = func.GetInitialPoint().block(0, 3, 3, 1),
		 goal = func.GetGoalPoint().block(0, 3, 3, 1);
	RenderManager& render = RenderManager::getInstance();
	render.ClearAuxiliaryObj();
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(ini),  0.010f, kRed));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(goal), 0.010f, kBlue));
	render.EmplaceAuxiliaryObj(std::make_shared<GLine>(Conversion::fromVec3f(ini), Conversion::fromVec3f(goal),
													   kRed, kBlue));
	func.SetTargetOptimizer(TargetOptimizer{});
	PlanningManager::getInstance().RegisterPlanningTask(robot, func, func.GetTimeTotal(), 40);
	
}

void PlanningSystem::ResponseCircleMotion(const QString& obj_name, float max_vel, float max_acc, float max_ang_vel, float max_ang_acc, std::vector<float> target_descarte, std::vector<float> waypoint)
{
	Model* robot = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_name));
	if (!robot)		return; // object not exist
	
	Eigen::Matrix4f goal_mat = STLUtils::toMat4f(target_descarte);
	Eigen::Vector3f mid_pos  = STLUtils::toVec3f(waypoint);
	CircMotion motion(goal_mat, mid_pos);
	motion.SetMaxLinVel(max_vel).SetMaxLinAcc(max_acc).
		   SetMaxAngVel(max_ang_vel).SetMaxAngAcc(max_ang_acc);

	Trajectory func = motion(robot);

	
	Vec3 ini  = func.GetInitialPoint().block(0, 3, 3, 1),
		 goal = func.GetGoalPoint().block(0, 3, 3, 1);	
	RenderManager& render = RenderManager::getInstance();
	render.ClearAuxiliaryObj();
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(ini),		0.010f, kRed));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(goal),		0.010f, kBlue));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(mid_pos),	0.010f, kPurple));
	auto samples_stl = func.GetSampleFromPathFunc(0.05f);
	std::vector<glm::vec3> samples(samples_stl.size());
	std::transform(samples_stl.begin(), samples_stl.end(), samples.begin(), STLUtils::toGlmVec3);
	render.EmplaceAuxiliaryObj(std::make_shared<GCurves>(samples, kRed, kBlue));

	PlanningManager::getInstance().RegisterPlanningTask(robot, func, func.GetTimeTotal(), 40);
}

void PlanningSystem::ResponseSplineMotion(const QString& obj_name, float max_vel, float max_acc, float max_ang_vel, float max_ang_acc, std::vector<float> target_descarte, std::vector<std::vector<float>> waypoints)
{
	Model* robot = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_name));
	if (!robot)		return; // object not exist
	Eigen::Matrix4f goal_mat = STLUtils::toMat4f(target_descarte);
	std::vector<Eigen::Vector3f> way_vecs(waypoints.size());
	std::transform(waypoints.begin(), waypoints.end(), way_vecs.begin(), STLUtils::toVec3f);
	SPlineMotion motion(goal_mat, way_vecs);
	motion.SetMaxLinVel(max_vel).SetMaxLinAcc(max_acc).
		   SetMaxAngVel(max_ang_vel).SetMaxAngAcc(max_ang_acc);

	Trajectory func = motion(robot);

	Vec3 ini  = func.GetInitialPoint().block(0, 3, 3, 1),
		 goal = func.GetGoalPoint().block(0, 3, 3, 1);
	RenderManager& render = RenderManager::getInstance();
	render.ClearAuxiliaryObj();
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(ini),	0.010f, kRed));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(goal), 0.010f, kBlue));
	for (auto& waypoint : way_vecs) {
		render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(waypoint), 0.010f, kPurple));
	}
	auto samples_stl = func.GetSampleFromPathFunc(0.05f);
	std::vector<glm::vec3> samples(samples_stl.size());
	std::transform(samples_stl.begin(), samples_stl.end(), samples.begin(), STLUtils::toGlmVec3);
	render.EmplaceAuxiliaryObj(std::make_shared<GCurves>(samples, kRed, kBlue));

	PlanningManager::getInstance().RegisterPlanningTask(robot, func, func.GetTimeTotal(), 40);
}

void PlanningSystem::ResponseDualSyncLineMotion(const std::vector<QString>& obj_names, std::vector<float> max_vels, std::vector<float> max_accs, std::vector<float> target, std::vector<std::vector<float>> bias)
{
	vector<Model*> robots(2);
	robots[0] = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_names[0]));
	robots[1] = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_names[1]));
	Eigen::Matrix4f goal_mat  = STLUtils::toMat4f(target),
					left_bias = STLUtils::toMat4f(bias[0]),
					right_bias= STLUtils::toMat4f(bias[1]);

	DualSyncLineMotion motion(goal_mat);
	motion.SetMaxLeftVel(max_vels[0]).SetMaxLeftAcc(max_accs[0]).
		   SetMaxLeftVel(max_vels[1]).SetMaxLeftAcc(max_accs[1]);
	motion.SetLeftTransform(left_bias).SetRightTransform(right_bias);

	std::vector<JTrajFunc> funcs; 
	auto [l_func, r_func] = motion(robots[0], robots[1]);
	funcs.emplace_back(l_func);
	funcs.emplace_back(r_func);
	
	Vec3 l_ini  = l_func.GetInitialPoint().block(0, 3, 3, 1),
		 l_goal = l_func.GetGoalPoint().block(0, 3, 3, 1),
		 r_ini  = r_func.GetInitialPoint().block(0, 3, 3, 1),
		 r_goal = r_func.GetGoalPoint().block(0, 3, 3, 1);
	RenderManager& render = RenderManager::getInstance();
	render.ClearAuxiliaryObj();
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(l_ini),  0.010f, kRed));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(l_goal), 0.010f, kBlue));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(r_ini),  0.010f, kRed));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(r_goal), 0.010f, kBlue));
	render.EmplaceAuxiliaryObj(std::make_shared<GLine>(Conversion::fromVec3f(l_ini), Conversion::fromVec3f(l_goal),
		kRed, kYellow));
	render.EmplaceAuxiliaryObj(std::make_shared<GLine>(Conversion::fromVec3f(r_ini), Conversion::fromVec3f(r_goal),
		kRed, kYellow));

	PlanningManager::getInstance().RegisterDualPlanningTask(robots, funcs, l_func.GetTimeTotal(), 40);
}

void PlanningSystem::ResponseDualSyncCircleMotion(const std::vector<QString>& obj_names, std::vector<float> max_vels, std::vector<float> max_accs, std::vector<float> target, std::vector<std::vector<float>> bias, std::vector<float> waypoint)
{
	vector<Model*> robots(2);
	robots[0] = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_names[0]));
	robots[1] = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_names[1]));
	Eigen::Matrix4f goal_mat  = STLUtils::toMat4f(target),
					left_bias = STLUtils::toMat4f(bias[0]),
					right_bias= STLUtils::toMat4f(bias[1]);
	Eigen::Vector3f way_vec   = STLUtils::toVec3f(waypoint);

	DualSyncCircleMotion motion(goal_mat, way_vec);
	motion.SetMaxLeftVel(max_vels[0]).SetMaxLeftAcc(max_accs[0]).
		   SetMaxLeftVel(max_vels[1]).SetMaxLeftAcc(max_accs[1]);
	motion.SetLeftTransform(left_bias).SetRightTransform(right_bias);

	std::vector<JTrajFunc> funcs;
	auto [l_func, r_func] = motion(robots[0], robots[1]);
	funcs.emplace_back(l_func);
	funcs.emplace_back(r_func);

	PlanningManager::getInstance().RegisterDualPlanningTask(robots, funcs, l_func.GetTimeTotal(), 40);
}

}
