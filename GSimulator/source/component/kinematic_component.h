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

#include <GComponent/grobotkinematic.h>

#include <optional>
#include <vector>

namespace GComponent {

using std::optional;
using std::vector;
template<class _Scalar>
using Thetas = vector<_Scalar>;
template<class _Scalar>
using Transforms = vector<Matrix<_Scalar, 4, 4>>;
template<class _Scalar>
using Thetav = Vector<_Scalar, -1>;
template<class _Scalar>
using Jacobi   = Matrix<_Scalar, Eigen::Dynamic, Eigen::Dynamic>;
template<class _Scalar>
using ZeroProj = Matrix<_Scalar, Eigen::Dynamic, Eigen::Dynamic>;

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
/// Kinematic related interface
	bool	  ForwardKinematic(SE3<float>&  out_mat);
	bool	  ForwardKinematic(SE3<float>&  out_mat, const Thetas<float>&  thetas);
	bool	  ForwardKinematic(SE3<float>&	out_mat, const Thetav<float>&  thetav);

	bool	  InverseKinematic(Thetas<float>&		 out_thetas,
							   const SE3<float>&	 trans_desire,
							   const Thetas<float>&  init_guess);
	bool	  InverseKinematic(Thetav<float>&		 out_thetas,
							   const SE3<float>&	 trans_desire,
							   const Thetav<float>&  init_guess);
	
	bool	  Jacobian(Jacobi<float>& out_mat, const Thetas<float>& thetas);
	bool	  Jacobian(Jacobi<float>& out_mat, const Thetav<float>& thetav);

	bool	  ZeroProjection(ZeroProj<float>& zero_mat, const Thetas<float>& thetas);
	bool	  ZeroProjection(ZeroProj<float>& zero_mat, const Thetav<float>& thetav);

	bool	  Transforms(vector<SE3<float>>& out_mats, const Thetas<float>& thetas);
	bool      Transforms(vector<SE3<float>>& out_mats, const Thetav<float>& thetav);

	bool	  DifferentialTransforms(vector<SE3<float>>& out_mats, const Thetas<float>& thetas);
	bool      DifferentialTransforms(vector<SE3<float>>& out_mats, const Thetav<float>& thetav);

	bool	  UpdateExponentialCoordinates();

/// Getter & Setter
	virtual const string_view& 
					GetTypeName() const override	{ return type_name; }

	inline void		SetIKSolver(IKSolverEnum solver_enum) 
													{ ik_solver_enum_ = solver_enum; }
	inline IKSolverEnum
					GetIKEnum()		const			{ return ik_solver_enum_; }
	inline unsigned GetJointCount() const			{ return joint_count_; }
	
	inline vector<float>   
					GetJointsPos() const			{ return GetJointsGroup()->GetPositions(); }

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
	void			tickImpl(float delta_time)		 override;
	QJsonObject		Save()							 override;
	bool		    Load(const QJsonObject& com_obj) override;

private:
	void			InitializeIKSolvers();
	inline JointGroupComponent* 
					GetJointsGroup() const { return ptr_parent_->GetComponent<JointGroupComponent>(); }

	Thetav<float>	toThetav(const Thetas<float> thetas);
	Thetas<float>	fromThetav(const Thetav<float> thetav);

private:
	unsigned				joint_count_		  = 0;
	vector<Twist<float>>	exp_coords_			  = {};
	SE3<float>				end_transform_mat_	  = SE3<float>::Identity();

	IKSolverEnum			ik_solver_enum_		  = IKSolverEnum::LeastNorm;
	RobotKinematic::IKSolver<float>*
							solver_				  = nullptr;
	double					precision_			  = 1e-5f;
	int						max_iteration_		  = 50;
	double					decay_scaler_		  = 0.3f;
	IKSolverTable			ik_solvers_			  = {};

/*________________________________Static Fields______________________________________________________________________*/						
public:
	constexpr static const	string_view		type_name = "KinematicComponent";
};
}

#endif // !_KINEMATIC_COMPONENT

