/**
 *  @file  	kinematic_component.h
 *  @brief 	This class is used for an abstract about robot kinematic.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	Jun 6th, 2022
 **/
#ifndef _KINEMATIC_COMPONENT
#define _KINEMATIC_COMPONENT

#include "component/component.hpp"
#include "component/joint_group_component.h"

#include "model/model.h"

#include <GComponent/GTransform.hpp>
#include <GComponent/grobotkinematic.h>

#include <optional>
#include <vector>

namespace GComponent {

using std::optional;
using std::vector;
template<class _Scaler>
using Thetas = vector<_Scaler>;
template<class _Scaler>
using Jacobian = Matrix<_Scaler, 6, -1>;
template<class _Scaler>
using Transforms = vector<Matrix<_Scaler, 4, 4>>;
template<class _Scaler>
using Thetav = Vector<_Scaler, -1>;

enum class IKSolverEnum {
	LeastNorm						= 0,
	DampedLeastSquare				= 1,
	JacobianTranspose				= 2,
	AdaptiveDampedLeastSquare		= 3,
	SelectivelyDampedLeastSquare	= 4
};

class KinematicComponent :public Component{
public:	
	using IKSolverTable = unordered_map <IKSolverEnum, unique_ptr<RobotKinematic::IKSolver<float>>>;

public:
	explicit KinematicComponent(Model* ptr_parent = nullptr);
	explicit KinematicComponent(const SE3<float>& initial_end_transform, Model* ptr_parent = nullptr);
	~KinematicComponent() = default;
	
	bool	  ForwardKinematic(SE3<float>&  out_mat, const Thetas<float>&  thetas);
	bool	  ForwardKinematic(SE3<float>&	out_mat, const Thetav<float>&  thetav);

	bool	  InverseKinematic(Thetas<float>&		 out_thetas,
							   const SE3<float>&	 trans_desire,
							   const Thetas<float>&  init_guess);
	bool	  InverseKinematic(Thetav<float>&		 out_thetas,
							   const SE3<float>&	 init_guess,
							   const Thetav<float>&  trans_desire);

	bool	  UpdateExponentialCoordinates();

/// Getter & Setter
	virtual const string_view& 
					GetTypeName() const override	{ return type_name; }
	// TODO: using enum not direct pointer
	inline void		SetIKSolver(IKSolverEnum solver_enum) 
													{ ik_solver_enum_ = solver_enum; }
	inline IKSolverEnum
					GetIKEnum()		const			{ return ik_solver_enum_; }
	inline unsigned GetJointCount() const			{ return joint_count_; }

	inline void		SetPrecision(double prececion)	{ precision_ = prececion; }
	inline double	GetPrecision()	const			{ return precision_; }

	inline void		SetMaxIteration(int max_limit)  { max_iteration_ = max_limit; }
	inline int		GetMaxIteration() const			{ return max_iteration_; }

	inline void		SetDecayScaler(double decay)	{ decay_scaler_ = decay; }
	inline double	GetDecayScaler() const			{ return decay_scaler_; }

	inline void		SetEndTransformInit(const SE3<float>& ini_mat) 
													{ end_transform_mat_ = ini_mat; }
	inline SE3<float>
					GetEndTransformInit() const		{ return end_transform_mat_; }

	inline void		SetExpCoords(const vector<Twist<float>>& exp_coords) { exp_coords_ = exp_coords; }
protected:
	void			tickImpl(float delta_time)	override;

private:
	void			InitializeIKSolvers();
	inline JointGroupComponent* 
					GetJointsGroup() { return ptr_parent_->GetComponent<JointGroupComponent>("JointGroupComponent"); }

	Thetav<float>			toThetav(const Thetas<float> thetas);
	Thetas<float>			fromThetav(const Thetav<float> thetav);

private:
	unsigned				joint_count_		  = 0;
	vector<Twist<float>>	exp_coords_			  = {};
	SE3<float>				end_transform_mat_	  = SE3<float>::Identity();

	IKSolverEnum			ik_solver_enum_		  = IKSolverEnum::LeastNorm;
	RobotKinematic::IKSolver<float>*
							solver_				  = nullptr;
	double					precision_			  = 1e-6f;
	int						max_iteration_		  = 50;
	double					decay_scaler_		  = 0.3f;
	IKSolverTable			ik_solvers_			  = {};

/*________________________________Static Fields______________________________________________________________________*/						
public:
	constexpr static const	string_view		type_name = "KinematicComponent";
};
}

#endif // !_KINEMATIC_COMPONENT

