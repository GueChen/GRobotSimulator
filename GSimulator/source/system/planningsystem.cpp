#include "system/planningsystem.h"

#include "manager/planningmanager.h"
#include "manager/modelmanager.h"
#include "manager/rendermanager.h"

#include "component/transform_component.h"
#include "component/kinematic_component.h"

#include "motion/GMotion"
#include "motion/optimization/skinsensor_optimizer.h"


#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG

#ifdef _LOG_RECORD
#include "system/loggersystem.h"

namespace GComponent{
static void LogEndPos(MotionBase& motion, Model& robot, Trajectory & func) {
	KinematicComponent& kine = *robot.GetComponent<KinematicComponent>();
	TransformCom& trans = *robot.GetTransform();
	float tot = motion.GetTotalTime(), interval = std::max(1e-15f, tot / 100.0f);
	if (isnan(tot) || isinf(tot)) return;

	for (float i = 0.0f; i <= tot; i += interval) {
		auto ret = func(i);
		SE3f mat; kine.ForwardKinematic(mat, ret.first);
		Vec3f pos = (trans.GetModelMatrixWithoutScale() * mat).block(0, 3, 3, 1);
		std::string msg = std::format("end_pos,{:},{:},{:},{:}", i, pos.x(), pos.y(), pos.z());
		LoggerSystem::getInstance().Log(LoggerObject::File, msg);
	}
}

static void LogEndPosDual(Model& l_robot, Trajectory& l_func, Model& r_robot, Trajectory& r_func) {
	KinematicComponent& l_kine = *l_robot.GetComponent<KinematicComponent>(),
					  & r_kine = *r_robot.GetComponent<KinematicComponent>();
	TransformCom& l_trans = *l_robot.GetTransform(),
		     & r_trans = *r_robot.GetTransform();
	float tot = l_func.GetTimeTotal(), interval = std::max(1e-15f, tot / 100.0f);
	if (isnan(tot) || isinf(tot)) return;
	for (float i = 0.0f; i <= tot; i += interval) {
		{	auto ret = l_func(i);
			SE3f mat; l_kine.ForwardKinematic(mat, ret.first);
			Vec3f pos = (l_trans.GetModelMatrixWithoutScale() * mat).block(0, 3, 3, 1);
			std::string msg = std::format("left_end_pos,{:},{:},{:},{:}", i, pos.x(), pos.y(), pos.z());
			LoggerSystem::getInstance().Log(LoggerObject::File, msg);
		}
		{
			auto ret = r_func(i);
			SE3f mat; r_kine.ForwardKinematic(mat, ret.first);
			Vec3f pos = (r_trans.GetModelMatrixWithoutScale() * mat).block(0, 3, 3, 1);
			std::string msg = std::format("right_end_pos,{:},{:},{:},{:}", i, pos.x(), pos.y(), pos.z());
			LoggerSystem::getInstance().Log(LoggerObject::File, msg);
		}
	}
}

}
#endif

namespace GComponent {

PlanningSystem& PlanningSystem::getInstance()
{
	static PlanningSystem instance;
	return instance;
}

PlanningSystem::~PlanningSystem() = default;

void PlanningSystem::BroadcastJointsAngle(const std::string& name, std::vector<float> joints, float time_stamp) {	
	static std::map<std::string, QString> obj_table = {
		{"aubo_i3_robot_0", "left"},
		{"aubo_i3_robot_1", "right"}
	};
	QString planning_obj;
	auto iter = obj_table.find(name);
	if (iter == obj_table.end()) {
		planning_obj = QString::fromStdString(name);
	}
	else {
		planning_obj = iter->second;
	}
	emit NotifyNewJointsAngle(planning_obj, joints, time_stamp);
}

void PlanningSystem::BroadcastTaskPause(const std::string& name)
{
	static std::map<std::string, QString> obj_table = {
		{"aubo_i3_robot_0", "left"},
		{"aubo_i3_robot_1", "right"}
	};	
	emit NotifyPauseTask(obj_table[name]);
}

/*___________________________________PROTECTED METHODS___________________________________________________*/
std::string PlanningSystem::SimpleGetObjName(const QString& obj_name)
{	
	if (obj_name == "Left") {
		return "aubo_i3_robot_0";
	}
	else if(obj_name == "Right") {
		return "aubo_i3_robot_1";
	}
	return obj_name.toStdString();
}

/*____________________________________PUBLIC SLOTS_______________________________________________________*/
void PlanningSystem::ResponsePTPMotionJSpace(const QString& obj_name, float max_vel, float max_acc, std::vector<float> target_joints)
{
	Model* robot = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_name));
	if (robot) {
		PTPMotion motion(target_joints);
		motion.SetMaxLinVel(max_vel).SetMaxLinAcc(max_acc);
		JTrajectory func = motion(robot);
		PlanningManager::getInstance().RegisterPlanningTask(new JTrajectory(func));
	
		KinematicComponent& kine	= *robot->GetComponent<KinematicComponent>();
		TransformCom		  & trans   = *robot->GetTransform();
		RenderManager     &	render	= RenderManager::getInstance();
		render.ClearAuxiliaryObj();
		std::vector<glm::vec3> samples;
		float tot = motion.GetTotalTime(), interval = std::max(1e-15f, tot / 20.0f);
		for (float i = 0.0f; i <= tot; i += interval) {
			auto ret = func(i);									
			SE3f mat; kine.ForwardKinematic(mat, ret.first);
			Vec3f pos = (trans.GetModelMatrixWithoutScale() * mat).block(0, 3, 3, 1);
			samples.push_back(Conversion::fromVec3f(pos));
		}
		if (samples.size() >= 2) {
			render.EmplaceAuxiliaryObj(std::make_shared<GCurves>(samples, kRed, kBlue));
			render.EmplaceAuxiliaryObj(std::make_shared<GBall>(samples.front(), 0.010f, kRed));
			render.EmplaceAuxiliaryObj(std::make_shared<GBall>(samples.back(), 0.010f, kBlue));
		}
	}	
	
	
}

//FIXME: THIS METHOD NOT GOOD FOR ISOLATION
// Because no class called PTPMotion descarte, therfore must inducting kinematic component
// Consider add a creator mehotd into PTPMotion for descarte target case
void PlanningSystem::ResponsePTPMotionCSpace(const QString& obj_name, float max_vel, float max_acc, std::vector<float> target_descarte)
{
	Model* robot = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_name));
	if (!robot)		return; // object not exist
	KinematicComponent* kine_sdk = robot->GetComponent<KinematicComponent>();
	if (!kine_sdk)	return; // kinematic com not exist
	TransformCom& trans = *robot->GetTransform();

	std::vector<float> target_joints;			
	Eigen::Matrix4f goal_mat = trans.GetModelMatrixWithoutScale().inverse() * STLUtils::toMat4f(target_descarte);
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
	JTrajectory func = motion(robot);
	PlanningManager::getInstance().RegisterPlanningTask(func.GetPtrCopy());
	
}

void PlanningSystem::ResponseLineMotion(const QString& obj_name, float max_vel, float max_acc, float max_ang_vel, float max_ang_acc, std::vector<float> target_descarte)
{
	Model* robot = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_name));
	if (!robot)		return; // object not exist
		
	Eigen::Matrix4f goal_mat = STLUtils::toMat4f(target_descarte);
	LineMotion motion(goal_mat);
	motion.SetMaxLinVel(max_vel).SetMaxLinAcc(max_acc).
		   SetMaxAngVel(max_ang_vel).SetMaxAngAcc(max_ang_acc);
	
	CTrajectory func = motion(robot);
	Vec3 ini  = func.GetInitialPoint().block(0, 3, 3, 1),
		 goal = func.GetGoalPoint().block(0, 3, 3, 1);

	TgtOptimizer* optimizer = nullptr;
	if (tgt_flag) {
		optimizer = new SkinSensorLineOptimizer((goal - ini).normalized());
	}
	func.SetTargetOptimizer(optimizer);
	func.SetSelfMotionOptimizer(GetSlfOptimizer());
	PlanningManager::getInstance().RegisterPlanningTask(func.GetPtrCopy());
	
	RenderManager& render = RenderManager::getInstance();
	render.ClearAuxiliaryObj();
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(ini),  0.010f, kRed));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(goal), 0.010f, kBlue));
	render.EmplaceAuxiliaryObj(std::make_shared<GLine>(Conversion::fromVec3f(ini), Conversion::fromVec3f(goal),
													   kRed, kBlue));
#ifdef _LOG_RECORD		
	LogEndPos(motion, *robot, func);	
#endif
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

	CTrajectory func = motion(robot);
	PlanningManager::getInstance().RegisterPlanningTask(func.GetPtrCopy());
	
	Vec3 ini  = func.GetInitialPoint().block(0, 3, 3, 1),
		 goal = func.GetGoalPoint().block(0, 3, 3, 1);	
	RenderManager& render = RenderManager::getInstance();
	render.ClearAuxiliaryObj();
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(ini),		0.010f, kRed));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(goal),		0.010f, kBlue));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(mid_pos),	0.010f, kPurple));
	auto samples_stl = func.GetSampleFromPathFunc(0.05f);
	std::vector<glm::vec3> samples(samples_stl.size());
	std::transform(samples_stl.begin(), samples_stl.end(), samples.begin(), Conversion::fromVec3f);
	render.EmplaceAuxiliaryObj(std::make_shared<GCurves>(samples, kRed, kBlue));

#ifdef _LOG_RECORD		
	LogEndPos(motion, *robot, func);
#endif
}

void PlanningSystem::ResponseSplineMotion(const QString& obj_name, const QString& spline_type, float max_vel, float max_acc, float max_ang_vel, float max_ang_acc, std::vector<float> target_descarte, std::vector<std::vector<float>> waypoints)
{
/*  mapping table */
	static std::map<QString, SplineType> type_entries = {
#define TypeNamePair(type) \
	{SplineName::type.data(), SplineType::type} 

		TypeNamePair(Cubic),
		TypeNamePair(Bezier),
		TypeNamePair(BezierInter),
		TypeNamePair(BSpline),
		TypeNamePair(BSplineInter),

#undef TypeNamePair

	};

	Model* robot = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_name));
	if (!robot)		return; // object not exist
	Eigen::Matrix4f goal_mat = STLUtils::toMat4f(target_descarte);
	std::vector<Eigen::Vector3f> way_vecs(waypoints.size());
	std::transform(waypoints.begin(), waypoints.end(), way_vecs.begin(), STLUtils::toVec3f);
	SPlineMotion motion(goal_mat, way_vecs, type_entries[spline_type]);
	motion.SetMaxLinVel(max_vel).SetMaxLinAcc(max_acc).
		   SetMaxAngVel(max_ang_vel).SetMaxAngAcc(max_ang_acc);

	CTrajectory func = motion(robot);
	PlanningManager::getInstance().RegisterPlanningTask(func.GetPtrCopy());

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
	std::transform(samples_stl.begin(), samples_stl.end(), samples.begin(), Conversion::fromVec3f);
	render.EmplaceAuxiliaryObj(std::make_shared<GCurves>(samples, kRed, kBlue));

#ifdef _LOG_RECORD
	LogEndPos(motion, *robot, func);
#endif
}

void PlanningSystem::ResponseKeeperMotion(const QString& obj_name, float time_total, std::vector<float> target_descarte)
{
	Model* robot = ModelManager::getInstance().GetModelByName(SimpleGetObjName(obj_name));
	if (!robot)		return; // object not exist
	Eigen::Matrix4f goal_mat = STLUtils::toMat4f(target_descarte);
	KeeperMotion motion(goal_mat, time_total);
	CTrajectory func = motion(robot);
	func.SetTargetOptimizer(
		//new SkinSensorOptimizer{}
		new SkinSensorSimpleKeeperOptimizer{}
	);
	PlanningManager::getInstance().RegisterPlanningTask(func.GetPtrCopy());

	RenderManager& render = RenderManager::getInstance();	
	render.ClearAuxiliaryObj();
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
	
	PlanningManager::getInstance().RegisterDualPlanningTask(robots, funcs, l_func.GetTimeTotal(), 5);

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

#ifdef _LOG_RECORD
	LogEndPosDual(*robots[0], l_func, *robots[1], r_func);
#endif
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

	PlanningManager::getInstance().RegisterDualPlanningTask(robots, funcs, l_func.GetTimeTotal(), 5);

	Vec3 l_ini  = l_func.GetInitialPoint().block(0, 3, 3, 1),
		 l_goal = l_func.GetGoalPoint().block(0, 3, 3, 1),
		 l_mid  = goal_mat.block(0, 0, 3, 3) * left_bias.block(0, 3, 3, 1) + way_vec;
	RenderManager& render = RenderManager::getInstance();
	render.ClearAuxiliaryObj();
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(l_ini), 0.010f, kRed));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(l_goal), 0.010f, kBlue));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(l_mid), 0.010f, kPurple));
	auto l_samples_stl = l_func.GetSampleFromPathFunc(0.05f);
	std::vector<glm::vec3> l_samples(l_samples_stl.size());
	std::transform(l_samples_stl.begin(), l_samples_stl.end(), l_samples.begin(), Conversion::fromVec3f);
	render.EmplaceAuxiliaryObj(std::make_shared<GCurves>(l_samples, kRed, kBlue));

	Vec3 r_ini  = r_func.GetInitialPoint().block(0, 3, 3, 1),
		 r_goal = r_func.GetGoalPoint().block(0, 3, 3, 1),
		 r_mid  = goal_mat.block(0, 0, 3, 3) * right_bias.block(0, 3, 3, 1) + way_vec;
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(r_ini), 0.010f, kRed));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(r_goal), 0.010f, kBlue));
	render.EmplaceAuxiliaryObj(std::make_shared<GBall>(Conversion::fromVec3f(r_mid), 0.010f, kPurple));
	auto r_samples_stl = r_func.GetSampleFromPathFunc(0.05f);
	std::vector<glm::vec3> r_samples(r_samples_stl.size());
	std::transform(r_samples_stl.begin(), r_samples_stl.end(), r_samples.begin(), Conversion::fromVec3f);
	render.EmplaceAuxiliaryObj(std::make_shared<GCurves>(r_samples, kRed, kBlue));

#ifdef _LOG_RECORD
	LogEndPosDual(*robots[0], l_func, *robots[1], r_func);
#endif
	
}

void PlanningSystem::ResponseChangeCurrentTaskStatus(const std::vector<QString>& obj_name, int status)
{
	for (auto& name : obj_name) {
		Model* robot = ModelManager::getInstance().GetModelByName(SimpleGetObjName(name));
		if (!robot) continue;
		PlanningManager::getInstance().ChangeCurrentTaskStatus(robot, status);
	}
}

void PlanningSystem::SetTargetOptimizer(int idx)
{
	enum TargetOptimizer : int {
		NONE = 0, PhysxChecker, SkinSensor
	}target_optimizer = static_cast<TargetOptimizer>(idx);

	switch (target_optimizer)
	{
	case PhysxChecker:	 tgt_flag = 1;	 qDebug() << "set phx";	    break;
	case SkinSensor:	 tgt_flag = 2;   qDebug() << "set skin";	break;
	default:			 tgt_flag = 0;	 qDebug() << "set null";	break;
	}
}

void PlanningSystem::SetSelfMotionOptimier(int idx)
{
	enum SelfOptimizer : int {
		NONE = 0, SelfMotion
	}self_optimizer = static_cast<SelfOptimizer>(idx);

	switch (self_optimizer)
	{
	case SelfMotion:	 slf_flag = 1;	 qDebug() << "set slfm";    break;
	default:			 slf_flag = 0;	 qDebug() << "set null";	break;
	}
}

TgtOptimizer* PlanningSystem::GetTgtOptimizer()
{
	switch (tgt_flag)
	{
	case 1:		return new PhysxCheckerOptimizer{};		break;
	case 2:		return new SkinSensorOptimizer{};		break;
	default:	return nullptr;							break;
	}
}

SlfOptimizer* PlanningSystem::GetSlfOptimizer()
{
	switch (slf_flag)
	{
	case 1:		return new SelfmotionOptimizer{};		break;
	default:	return nullptr;							break;
	}
}
}
