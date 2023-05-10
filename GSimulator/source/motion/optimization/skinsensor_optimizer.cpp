#include "motion/optimization/skinsensor_optimizer.h"

#include "system/skinsystem.h"

#include "model/model.h"
#include "component/transform_component.h"
#include "component/kinematic_component.h"
#include "manager/planningmanager.h"

#ifdef _DEBUG
#include <iostream>
#endif // _DEBUG


std::vector<float> GComponent::SkinSensorOptimizer::Optimize(Model& obj, const Twistf& glb_t, const std::vector<float>& thetas)
{
    std::vector<float> ret = thetas;
    std::vector<Eigen::Vector3f> hit_list = SkinSystem::getInstance().GetTriVector();
    if (!hit_list.empty()) {
        auto& kine = *obj.GetComponent<KinematicComponent>();
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

/*_____________________________Skin Sensor Simple Keeper Optimizer_____________________*/
std::vector<float> GComponent::SkinSensorSimpleKeeperOptimizer::Optimize(Model&obj, const Twistf& glb_t, const std::vector<float>& thetas)
{
    std::vector<float> ret = thetas;
    std::vector<Eigen::Vector3f> hit_list = SkinSystem::getInstance().GetTriVector();
    if (!hit_list.empty()) {
        TransformCom& trans_com = *obj.GetTransform();
        SE3f  glb     = trans_com.GetModelMatrixWithoutScale();
        SE3f  glb_inv = InverseSE3(glb);
        auto& kine    = *obj.GetComponent<KinematicComponent>();
        
        SE3f goal; kine.ForwardKinematic(goal, ret); goal = glb * goal;

        std::vector<SE3f> trans; kine.Transforms(trans, thetas);
        std::partial_sum(trans.begin(), trans.end(), trans.begin(), std::multiplies<>{});
        SO3<float> orientation = trans_com.GetModelMatrixWithoutScale().block(0, 0, 3, 3) * trans[5].block(0, 0, 3, 3);

        for (auto& vec : hit_list) {
            Vector3f move_dir = -orientation* vec;
            goal.block(0, 3, 3, 1) += move_dir * 0.002f;
#ifdef _DEBUG
            std::cout << "ori vec: " << vec.transpose() <<
                ", move dir : " << move_dir.transpose() << std::endl;
#endif // _DEBUG            
        }
        std::vector<float> new_pos;        
        if (kine.InverseKinematic(new_pos, glb_inv * goal, ret)) {
            ret = new_pos;            
        }
        
    }
    return ret;
}

bool GComponent::SkinSensorSimpleKeeperOptimizer::ConditionCheck(Model&)
{
    return SkinSystem::getInstance().Flag();
}


/*______________________________Skin Sensor Line Optimizer________________________*/
GComponent::SkinSensorLineOptimizer::SkinSensorLineOptimizer(Vec3 dir):
    fwd_dir_(dir)
{}

std::vector<float> GComponent::SkinSensorLineOptimizer::Optimize(Model& obj, const Twistf& glb_t, const std::vector<float>& thetas)
{
    std::vector<float> ret = thetas;
    std::vector<Eigen::Vector3f> vec_list = SkinSystem::getInstance().GetTriVector();    
    auto status = PlanningManager::getInstance().GetCurrentTaskStatus(&obj);
    if (!vec_list.empty()) {
        TransformCom& trans_com = *obj.GetTransform();
        SE3f  glb     = trans_com.GetModelMatrixWithoutScale();
        SE3f  glb_inv = InverseSE3(glb);
        auto& kine    = *obj.GetComponent<KinematicComponent>();
        SE3f  goal; kine.ForwardKinematic(goal, ret); goal = glb * goal;

        std::vector<SE3f> trans; kine.Transforms(trans, thetas);
        std::partial_sum(trans.begin(), trans.end(), trans.begin(), std::multiplies<>{});
//        SO3<float> orientation = obj.getModelMatrixWithoutScale().block(0, 0, 3, 3) * trans[5].block(0, 0, 3, 3);
        
        auto iter = std::find_if(vec_list.begin(), vec_list.end(), [&fwd_dir = this->fwd_dir_/*, &ori = orientation*/](auto && vec) {
            return  /*(ori * */vec.dot(fwd_dir) > 0.85;
        });
        
        if (iter != vec_list.end()) {  // exist orthorgnal direction and the angle smaller than 45                                    
            if (blocking_count_ == 0 && status != PlanningTask::eBlocking) {
                PlanningManager::getInstance().ChangeCurrentTaskStatus(&obj, PlanningTask::eBlocking);
                PlanningManager::getInstance().InteruptPrePlanning(&obj);
            }                                    
            //std::cout << "block here cur vec: " << (orientation * *iter).transpose() << std::endl;
            blocking_count_ = std::min(blocking_count_ + 1, 10);
        }
        else {
            Vec3 move_dir = Vec3::Zero();
            //std::cout << "the move dir: " << fwd_dir_.transpose() << std::endl;
            for (auto& vec : vec_list) {
                float cos_val = vec.dot(fwd_dir_);                
                move_dir += (vec - cos_val * fwd_dir_);
            }
            goal.block(0, 3, 3, 1) -= 0.01f * move_dir;            
            blocking_count_ = std::max(blocking_count_ - 2, 0);
            if (blocking_count_ == 0 && status != PlanningTask::eExecution) {                
                PlanningManager::getInstance().ChangeCurrentTaskStatus(&obj, PlanningTask::eExecution);
            }
        }
        
        if (vector<float> out; kine.InverseKinematic(out, glb_inv * goal, ret)) {
            ret = out;
            if (blocking_count_ == 0) {
                solve_failed_count_ = 0;
            }
        }
        else {
            ++solve_failed_count_;
        }
    }    
                  
    return ret;
}

bool GComponent::SkinSensorLineOptimizer::ConditionCheck(Model& obj)
{
    bool result = SkinSystem::getInstance().Flag();
    if (!result) {
        blocking_count_ = std::max(blocking_count_ - 2, 0);
        if (blocking_count_ == 0 && PlanningManager::getInstance().GetCurrentTaskStatus(&obj) != PlanningTask::eExecution) {
            PlanningManager::getInstance().ChangeCurrentTaskStatus(&obj, PlanningTask::eExecution);
        }
    }
    return result;
}
