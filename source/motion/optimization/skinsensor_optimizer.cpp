#include "motion/optimization/skinsensor_optimizer.h"

#include "system/skinsystem.h"

#include "model/model.h"
#include "component/kinematic_component.h"

#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG


std::vector<float> GComponent::SkinSensorOptimizer::Optimize(Model& obj, const Twistf& glb_t, const std::vector<float>& thetas)
{
    std::vector<float> ret = thetas;
    std::vector<Eigen::Vector3f> hit_list = SkinSystem::getInstance().GetTriVector();
    if (!hit_list.empty()) {
        auto& kine = *obj.GetComponent<KinematicComponent>(KinematicComponent::type_name.data());
        const int kN = kine.GetJointCount();
        std::vector<SE3f>  transforms; kine.Transforms(transforms, thetas);
        std::vector<SE3f>  difftrans;  kine.DifferentialTransforms(difftrans, thetas);
        std::partial_sum(transforms.begin(), transforms.end(), transforms.begin(), std::multiplies<>{});
        std::vector<SE3f> diff_mat(kN, SE3f::Zero());
        for (int i = 0; i < 6; ++i) {
            diff_mat[i] = i == 0 ? SE3f::Identity() : transforms[i - 1];
            diff_mat[i] = diff_mat[i] * difftrans[i] * InverseSE3(transforms[i]) * transforms[5];
        }

        DynMat<float> grad_mat = DynMat<float>::Zero(kN, 3);
        Vec3 checker_pos       = Vec3(0.0f, 0.0f, 1.10f);
        for (int i = 0; i < kN; ++i) {
            grad_mat.row(i) = AffineProduct(diff_mat[i], checker_pos).transpose();
        }

        DynVec<float> ret_grad_vec = DynVec<float>::Zero(kN);
        Mat3 orientation = 
            //kine.GetParent()->getModelMatrixWithoutScale().block(0, 0, 3, 3) * 
            transforms[5].block(0, 0, 3, 3);
        for (auto& vec : hit_list) {
            Vec3 rob_base = -/*orientation **/ vec;
            ret_grad_vec += 0.2f * grad_mat * ClampMaxAbs(Eigen::VectorX<float>(rob_base), 0.01);
#ifdef _DEBUG
            std::cout << "pene vec: " << rob_base.transpose() << std::endl;
#endif // _DEBUG
        }
        std::transform(ret.begin(), ret.end(), ret_grad_vec.begin(), ret.begin(), std::plus<>{});
#ifdef _DEBUG
        SE3f mat; kine.ForwardKinematic(mat, ret);
        std::cout << "opt bias: " << (ExpMapping(glb_t).block(0, 3, 3, 1) - mat.block(0, 3, 3, 1)).transpose() << std::endl;
#endif // _DEBUG        
    }
    return ret;
}

bool GComponent::SkinSensorOptimizer::ConditionCheck(Model&)
{
    //TODO: modify it with the sensor interface
    return SkinSystem::getInstance().Flag();
}

/*____________________________Find the Vector try to  move to orthogonal dir___________________________*/
std::vector<float> GComponent::SkinSensorSimpleOptimizer::Optimize(Model&obj, const Twistf& glb_t, const std::vector<float>& thetas)
{
    std::vector<float> ret = thetas;
    std::vector<Eigen::Vector3f> hit_list = SkinSystem::getInstance().GetTriVector();
    if (!hit_list.empty()) {
        SE3f  glb_inv = obj.getModelMatrixWithoutScale().inverse();
        auto& kine    = *obj.GetComponent<KinematicComponent>(KinematicComponent::type_name.data());
        
        SE3f goal; kine.ForwardKinematic(goal, ret);
        goal = kine.GetParent()->getModelMatrix() * goal;
        for (auto& vec : hit_list) {
            Vector3f move_dir = Roderigues(0.5f * MyPI * Vector3f::UnitZ()) * vec;
            goal.block(0, 3, 3, 1) += move_dir * 0.002f;
            std::cout << "ori vec: " << vec.transpose() << 
                         ", move dir : " << move_dir.transpose() << std::endl;
        }
        std::vector<float> new_pos;
        //Twistf new_goal = LogMapSE3Tose3(goal);
        if (kine.InverseKinematic(new_pos, glb_inv * goal, ret)) {
            ret = new_pos;            
        }
        
    }
    return ret;
}

bool GComponent::SkinSensorSimpleOptimizer::ConditionCheck(Model&)
{
    return SkinSystem::getInstance().Flag();
}
