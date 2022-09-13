#include "motion/optimization/skinsensor_optimizer.h"

#include "model/model.h"
#include "component/kinematic_component.h"

std::vector<float> GComponent::SkinSensorOptimizer::Optimize(Model& obj, const Twistf& glb_t, const std::vector<float>& thetas)
{
    std::vector<float> ret = thetas;
    std::vector<Eigen::Vector3f> hit_list;// = sensor::get();
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
        Vec3 checker_pos       = Vec3(0.0f, 0.0f, 1.25f);
        for (int i = 0; i < kN; ++i) {
            grad_mat.row(i) = AffineProduct(diff_mat[i], checker_pos).transpose();
        }

        DynVec<float> ret_grad_vec = DynVec<float>::Zero(kN);
        Mat3 orientation = transforms[5].block(0, 0, 3, 3);
        for (auto& vec : hit_list) {
            Vec3 rob_base = -orientation * vec;
            ret_grad_vec += 0.2f * grad_mat * ClampMaxAbs(Eigen::VectorX<float>(rob_base), 0.03);
        }
        std::transform(ret.begin(), ret.end(), ret_grad_vec.begin(), ret.begin(), std::plus<>{});
    }
    return ret;
}

bool GComponent::SkinSensorOptimizer::ConditionCheck(Model&)
{
    //TODO: modify it with the sensor interface
    return false;
}
