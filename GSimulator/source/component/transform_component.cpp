#include "component/transform_component.h"
#include "model/model.h"

#include "base/json_serializer.hpp"

namespace GComponent{
TransformComponent::TransformComponent(Model* ptr_parent, const Matrix4x4& mat):
    Component(ptr_parent)
{
    SetModelLocal(mat);
}

TransformComponent::TransformComponent(Model* ptr_parent, const Vector3& pos, const Vector3& rot, const Vector3& scale):
    Component(ptr_parent),
    position_(pos),
    rotation_(rot),
    scale_(scale)
{
    UpdateModelMatrix();
}

void TransformComponent::SetModelLocal(const Matrix4x4& mat, bool update)
{
    std::tie(position_, rotation_, scale_, shear_) = TRSSDecompositionMat4(mat);
    if (update) {
        UpdateModelMatrix();
    }
}

Matrix4x4 TransformComponent::GetTranslationModelMatrix() const
{
    Matrix4x4 model_mat = GetModelGlobal();
    model_mat.block(0, 0, 3, 3) = Matrix3x3::Identity();    
    return model_mat;
}

Matrix4x4 TransformComponent::GetModelMatrixWithoutScale() const
{
    Eigen::Affine3f model_trans_rot;
    model_trans_rot.setIdentity();
    model_trans_rot.translate(position_);
    if (rotation_.norm() > 1e-6)
    {
        model_trans_rot.rotate(Eigen::AngleAxisf(rotation_.norm(), rotation_ / rotation_.norm()));
    }
    return parent_mat_ * model_trans_rot.matrix();
}

void TransformComponent::SetTransLocal(const Vector3& translation, bool updateflag)
{
    position_ = translation;
    if (updateflag)
    {
        UpdateModelMatrix();
    }
}

Vector3 TransformComponent::GetTransGlobal() const
{        
    return AffineProduct(parent_mat_, position_);
}

void TransformComponent::SetTransGlobal(const Vector3& translation, bool update)
{
    Vector3 new_local = AffineProduct(InverseSE3(parent_mat_), translation);
    SetTransLocal(new_local, update);
}

void TransformComponent::SetRotLocal(const Vector3& rotation, bool updateflag)
{
    rotation_ = rotation;
    if (updateflag)
    {
        UpdateModelMatrix();
    }
}

Vector3 TransformComponent::GetRotGlobal() const
{
    Matrix3x3 rot_mat = GetModelMatrixWithoutScale().block(0, 0, 3, 3);
    return LogMapSO3Toso3(rot_mat);
}

void TransformComponent::SetRotGlobal(const Vector3& rotation, bool update)
{
    Matrix3x3 rot_mat = parent_mat_.block(0, 0, 3, 3).transpose()* Roderigues(rotation);
    Vector3   new_rot = LogMapSO3Toso3(rot_mat);
    SetRotLocal(new_rot);
}

void TransformComponent::SetScale(const Vector3 scale, bool updateflag)
{
    scale_ = scale;
    if (updateflag)
    {
        UpdateModelMatrix();
    }
}

void TransformComponent::UpdateModelMatrix()
{
     Eigen::Affine3f af_model; 
     af_model.setIdentity();

     Matrix4x4 scale_part = Matrix4x4::Identity(),
               shear_part = Matrix4x4::Identity();    
    
    // translation
    af_model.translate(position_);

    // rotation
    if (rotation_.norm() > 1e-6) {
        af_model.rotate(Eigen::AngleAxisf(rotation_.norm(), rotation_ / rotation_.norm()));
    }    
    
    // scale part
    if ((scale_ - Vector3::Ones()).norm() > 1e-6){
        scale_part.block(0, 0, 3, 3) = Scale(scale_);
    }

    // shear part
    if (shear_.norm() > 1e-6){
        shear_part.block(0, 0, 3, 3) = Shear(shear_);
    }

    model_mat_ = af_model.matrix() * scale_part * shear_part;
    
    UpdateChildrenMatrix(Matrix3x3::Identity()
    /*scale_part.block(0, 0, 3, 3) * shear_part.block(0, 0, 3, 3)*/
    );

    is_dirty_ = true;
}

void TransformComponent::UpdateChildrenMatrix(const Matrix3x3& parent_scale_mat)
{    
    for(auto & child : ptr_parent_->getChildren())
    {    
        TransformComponent& child_trans = *child->GetComponent<TransformComponent>();
        Vector3& child_pos    = child_trans.position_,
               & child_rot    = child_trans.rotation_,
               & child_scl    = child_trans.scale_,
               & child_shr    = child_trans.shear_;
        Matrix3x3& inv_U      = child_trans.inv_parent_U_;
        Matrix4x4& parent_mat = child_trans.parent_mat_;

        auto [Q_ori, R_ori] = QRDecompositionMat3((inv_U * Roderigues(child_rot)).eval());
        auto [Q_new, R_new] = QRDecompositionMat3((parent_scale_mat * Q_ori).eval());
        Mat3 new_U = R_new * R_ori;
        
        if ((child_scl - Vec3::Ones()).norm() > 1e-6) {
            new_U = new_U * Scale(child_scl);
        }
        if (child_shr.norm() > 1e-6) {
            new_U = new_U * Shear(child_shr);
        }
        
        parent_mat = GetModelMatrixWithoutScale();
        child_pos  = parent_scale_mat * inv_U * child_pos;
        child_rot  = LogMapSO3Toso3(Q_new);        
        inv_U = parent_scale_mat.inverse();
        if (!std::isnan(new_U(0, 0))) {
            std::tie(child_scl, child_shr) = SSDecompositionMat3(new_U);
        }
        child_trans.UpdateModelMatrix();        
    }
}

void TransformComponent::tickImpl(float delta)
{}

const string_view& TransformComponent::GetTypeName() const
{
    return type_name;    
}

QJsonObject TransformComponent::Save()
{
    QJsonObject com_obj;
    com_obj["type"] = type_name.data();

    com_obj["position"] = JSonSerializer::Serialize(position_);
    com_obj["rotation"] = JSonSerializer::Serialize(rotation_);
    com_obj["scale"]    = JSonSerializer::Serialize(scale_);
    com_obj["shear"]    = JSonSerializer::Serialize(shear_);

    com_obj["parent_mat"]   = JSonSerializer::Serialize(parent_mat_);
    com_obj["model_mat"]    = JSonSerializer::Serialize(model_mat_);
    com_obj["inv_parent_U"] = JSonSerializer::Serialize(inv_parent_U_);    

    com_obj["is_dirty"]  = is_dirty_;

    return com_obj;
}

bool TransformComponent::Load(const QJsonObject& com_obj)
{
    position_ = JsonDeserializer::ToVec3f(com_obj["position"].toArray());
    rotation_ = JsonDeserializer::ToVec3f(com_obj["rotation"].toArray());
    scale_    = JsonDeserializer::ToVec3f(com_obj["scale"].toArray());
    shear_    = JsonDeserializer::ToVec3f(com_obj["shear"].toArray());

    parent_mat_   = JsonDeserializer::ToMat4f(com_obj["parent_mat"].toArray());
    model_mat_    = JsonDeserializer::ToMat4f(com_obj["model_mat"].toArray());
    inv_parent_U_ = JsonDeserializer::ToMat3f(com_obj["inv_parent_U"].toArray());

    is_dirty_ = com_obj["is_dirty"].toBool();

    return true;
}



}