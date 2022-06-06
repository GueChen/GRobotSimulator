#include "model.h"

#include "manager/rendermanager.h"
#include "render/myshader.h"

#ifdef _DUBUG
#include <iostream>
#include <format>
#endif

//#define _DEBUG

using namespace GComponent;
using Translation3f = Eigen::Translation3f;
using AngleAxisf    = Eigen::AngleAxisf;

Model::Model(_RawPtr parent, const string & meshKey):
    mesh_(meshKey)
{
    if (parent) {
        parent->appendChild(this, model_mat_);
    }
}

Model::Model(const string & name, const string & mesh, const string & shader,const Mat4 & model_mat, _RawPtr parent) :
    name_(name), mesh_(mesh), shader_(shader)
{
    setModelMatrix(model_mat);  
    if (parent) {
        parent->appendChild(this, model_mat_);
    }
}

GComponent::Model::Model(const string & name,  const string & mesh, const string & shader, 
                         const Vec3   & trans, const Vec3 & rot, const Vec3 & scale, _RawPtr parent):
    name_(name), mesh_(mesh), shader_(shader),
    trans_(trans), rot_(rot), scale_(scale)
{    
    updateModelMatrix();
    if (parent) {
        parent->appendChild(this, model_mat_);
    }    
}

Model::~Model() 
{
    if (parent_) 
    {
        parent_->eraseChild(parent_->getChildIndex(this));
    }    
    parent_ = nullptr;
}

void GComponent::Model::tick(float delta_time)
{
    tickImpl(delta_time);
    for (auto& component : components_ptrs_) {
        component->tick(delta_time);
    }
}

void GComponent::Model::tickImpl(float delta_time)
{
    RenderManager::getInstance().EmplaceRenderCommand(name_, shader_, mesh_);    
}

void Model::Draw(MyShader* shader)
{
    shader->setMat4("model", Conversion::fromMat4f(getModelMatrix()));
    /* Draw With Source */
}

bool GComponent::Model::eraseChild(int idx)
{
    if (idx < 0 || idx >= children_.size()) return false;
    children_.erase(children_.begin() + idx);
    return true;
}

void Model::setModelMatrix(const Mat4 &mat)
{            
    std::tie(trans_, rot_, scale_, shear_) = TRSSDecompositionMat4(mat);    
    updateModelMatrix();
}

Mat4 GComponent::Model::getTranslationModelMatrix() const
{
    Mat4 model_mat = getModelMatrix();
    model_mat.block(0, 0, 3, 3) = Mat3::Identity();
    return model_mat;
}

Mat4 GComponent::Model::getModelMatrixWithoutScale() const
{
    Eigen::Affine3f model_trans_rot;
    model_trans_rot.setIdentity();
    model_trans_rot.translate(trans_);
    if (rot_.norm() > 1e-6)
    {
        model_trans_rot.rotate(AngleAxisf(rot_.norm(), rot_ / rot_.norm()));
    }
    return parent_model_mat_ * model_trans_rot.matrix();
}

void GComponent::Model::setTransLocal(const Vec3& translation, bool updateflag)
{
    trans_ = translation;
    if (updateflag)
    {
        updateModelMatrix();
    }
}

Vec3 GComponent::Model::getTransGlobal() const
{
    Mat4 model_mat = getModelMatrix();
    return Vec3(model_mat(0, 3), model_mat(1,3), model_mat(2, 3));
}

void GComponent::Model::setRotLocal(const Vec3& rotation, bool updateflag)
{
    rot_ = rotation;
    if (updateflag)
    {
        updateModelMatrix();
    }
}

Vec3 GComponent::Model::getRotGlobal() const
{ 
    return LogMapSO3Toso3(static_cast<Eigen::Matrix3f>(getModelMatrixWithoutScale().block(0, 0, 3, 3)));
}

void GComponent::Model::setScale(const Vec3 scale, bool updateflag)
{
    scale_ = scale;
    if (updateflag) 
    {
        updateModelMatrix();
    }
}

bool GComponent::Model::RegisterComponent(_PtrComponent&& component_ptr)
{
    // TODO: Add a hash Set to fillter the same type component
    component_ptr->SetParent(this);
    components_type_names_.push_back(component_ptr->GetTypeName().data());
    components_ptrs_.push_back(std::move(component_ptr));    
    return true;
}

void Model::appendChild(const _RawPtr pchild, Mat4 transform)
{
    pchild->setParent(this);
    children_.emplace_back(pchild);
    pchild->setModelMatrix(transform);              
    updateChildrenMatrix(Mat3::Identity()/*Scale(scale_)* Shear(shear_)*/);
}

int GComponent::Model::getChildIndex(_RawPtr ptr)
{
    for (int idx = 0; auto & ptr_child : children_) {
        if (ptr_child == ptr) {
            return idx;
        }
        ++idx;
    }
    return -1;
}

void GComponent::Model::updateModelMatrix()
{           
    Eigen::Affine3f af_model;
    af_model.setIdentity();
    af_model.translate(trans_);
    if (rot_.norm() > 1e-6) {
        af_model.rotate(AngleAxisf(rot_.norm(), rot_ / rot_.norm()));
    }    
    Mat4 scale_part = Mat4::Identity(),
         shear_part = Mat4::Identity();
    if ((scale_ - Vec3::Ones()).norm() > 1e-6)
    {
        scale_part.block(0, 0, 3, 3) = Scale(scale_);
    }
    if (shear_.norm() > 1e-6)
    {
        shear_part.block(0, 0, 3, 3) = Shear(shear_);
    }
    model_mat_ = af_model.matrix() * scale_part * shear_part;
    updateChildrenMatrix(Mat3::Identity()/*scale_part.block(0, 0, 3, 3) * shear_part.block(0, 0, 3, 3)*/);
}

void Model::updateChildrenMatrix(const Mat3& parent_adjoint_mat)
{    
    for(auto & child : children_)
    {                            
        auto [Q_ori, R_ori]          = QRDecompositionMat3((child->inv_parent_U_mat_ * Roderigues(child->rot_)).eval());
        auto [Q_new, R_new]          = QRDecompositionMat3((parent_adjoint_mat * Q_ori).eval());
        Mat3 upper_triangle_mat_new  = R_new * R_ori;
        if ((child->scale_ - Vec3::Ones()).norm() > 1e-6) {
            upper_triangle_mat_new = upper_triangle_mat_new * Scale(child->scale_);
        }
        if (child->shear_.norm() > 1e-6) 
        {
            upper_triangle_mat_new = upper_triangle_mat_new * Shear(child->shear_);
        }

        child->parent_model_mat_     = getModelMatrixWithoutScale();
        child->trans_                = parent_adjoint_mat * child->inv_parent_U_mat_ * child->trans_;
        child->rot_                  = LogMapSO3Toso3(Q_new);        
        std::tie(child->scale_, child->shear_)
                                     = SSDecompositionMat3(upper_triangle_mat_new);
        child->inv_parent_U_mat_     = parent_adjoint_mat.inverse();

        child->updateModelMatrix();        
    }
}

void GComponent::Model::setShaderProperty(MyShader& shader)
{
    shader.setMat4("model", Conversion::fromMat4f(getModelMatrix()));
    //shader.setVec3("color", glm::Vec3(1.0f));
}
