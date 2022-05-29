#include "model.h"

#include "manager/rendermanager.h"
#include "render/myshader.h"

#ifdef _DUBUG
#include <iostream>
#include <format>
#endif

//#define _DEBUG

using namespace GComponent;

Model::Model(_RawPtr parent, const string & meshKey):
    mesh_(meshKey)
{
    if (parent) {
        parent->appendChild(this, model_mat_);
    }
}

Model::Model(const string & name, const string & mesh, const string & shader,const mat4 & model_mat, _RawPtr parent) :
    name_(name), mesh_(mesh), shader_(shader)
{
    setModelMatrix(model_mat);  
    if (parent) {
        parent->appendChild(this, model_mat_);
    }
}

GComponent::Model::Model(const string & name,  const string & mesh, const string & shader, 
                         const vec3   & trans, const vec3 & rot, const vec3 & scale, _RawPtr parent):
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
    shader->setMat4("model", getModelMatrix());
    /* Draw With Source */
}

bool GComponent::Model::eraseChild(int idx)
{
    if (idx < 0 || idx >= children_.size()) return false;
    children_.erase(children_.begin() + idx);
    return true;
}

void Model::setModelMatrix(const mat4 &mat)
{            
    auto [trans, rot, scale, shear] = TRSSDecompositionMat4(Conversion::toMat4f(mat));
    trans_     = Conversion::fromVec3f(trans);
    rot_       = Conversion::fromVec3f(rot);
    scale_     = Conversion::fromVec3f(scale);   
    shear_     = Conversion::fromVec3f(shear);
    updateModelMatrix();
}

mat4 GComponent::Model::getTranslationModelMatrix() const
{
    mat4 model_mat = getModelMatrix();
    return mat4(1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                model_mat[3][0], model_mat[3][1], model_mat[3][2], 1.0f);
}

mat4 GComponent::Model::getModelMatrixWithoutScale() const
{
    mat4 model_mat = parent_model_mat_;
    model_mat = glm::translate(model_mat, trans_);
    if (float angle = glm::l2Norm(rot_); angle > 1e-6)
    {
        model_mat = glm::rotate(model_mat, angle, rot_ / angle);
    }
    return model_mat;
}

void GComponent::Model::setTransLocal(const vec3& translation, bool updateflag)
{
    trans_ = translation;
    if (updateflag)
    {
        updateModelMatrix();
    }
}

vec3 GComponent::Model::getTransGlobal() const
{
    mat4 model_mat = getModelMatrix();
    return vec3(model_mat[3][0], model_mat[3][1], model_mat[3][2]);
}

void GComponent::Model::setRotLocal(const vec3& rotation, bool updateflag)
{
    rot_ = rotation;
    if (updateflag)
    {
        updateModelMatrix();
    }
}

vec3 GComponent::Model::getRotGlobal() const
{ 
    return Conversion::fromVec3f(LogMapSO3Toso3(static_cast<Eigen::Matrix3f>(Conversion::toMat4f(getModelMatrixWithoutScale()).block(0, 0, 3, 3))));
}

void GComponent::Model::setScale(const vec3 scale, bool updateflag)
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

void Model::appendChild(const _RawPtr pchild, mat4 transform)
{
    pchild->setParent(this);
    pchild->setModelMatrix(transform);    
    children_.emplace_back(pchild);
    updateChildrenMatrix(glm::scale(mat4(1.0f), scale_));            
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
    mat4 trans_mat = glm::translate(mat4(1.0f), trans_);
       
    mat4 rot_mat(1.0f);        
    if (float angle = glm::l2Norm(rot_); angle > 1e-8)
    {
        rot_mat = glm::rotate(rot_mat, angle, rot_ / angle);        
    }

    mat4 scale_mat(1.0f);
    if (abs(glm::l2Norm(scale_ - vec3(1.0f))) > 1e-4) 
    {
        scale_mat = glm::scale(scale_mat, scale_);
    }        
    mat4 shear_mat(1.0f);
    if (glm::l2Norm(shear_) > 1e-4) {
        shear_mat[1][0] = shear_.x;
        shear_mat[2][0] = shear_.y;
        shear_mat[2][1] = shear_.z;
    }
       
    model_mat_ = trans_mat * rot_mat * scale_mat * shear_mat;
    updateChildrenMatrix(scale_mat * shear_mat);
}

void Model::updateChildrenMatrix(const mat4& parent_adjoint_mat)
{    
    for(auto & child : children_)
    {                    
        Eigen::Vector3f rot_old      = Conversion::toVec3f(child->rot_),
                        trans_old    = Conversion::toVec3f(child->trans_);
        Eigen::Matrix3f scale_mat    = Scale(Conversion::toVec3f(child->scale_)),
                        shear_mat    = Shear(Conversion::toVec3f(child->shear_));
        Eigen::Matrix3f inv_p_adj_R_ = Conversion::toMat3f(child->inv_parent_U_mat_),
                        p_adj_R_new  = Conversion::toMat4f(parent_adjoint_mat).block(0, 0, 3, 3);
                       
        auto [Q_ori, R_ori]          = QRDecompositionMat3((inv_p_adj_R_ * Roderigues(rot_old)).eval());
        auto [Q_new, R_new]          = QRDecompositionMat3((p_adj_R_new * Q_ori).eval());       
        
        Eigen::Vector3f rot          = LogMapSO3Toso3(Q_new),
                        trans        = p_adj_R_new * inv_p_adj_R_ * trans_old,
                        scale,
                        shear;
        std::tie(scale, shear)       = SSDecompositionMat3((R_new * R_ori * scale_mat * shear_mat).eval());

        child->parent_model_mat_     = getModelMatrixWithoutScale();
        child->trans_                = Conversion::fromVec3f(trans);
        child->rot_                  = Conversion::fromVec3f(rot);
        child->scale_                = Conversion::fromVec3f(scale);
        child->shear_                = Conversion::fromVec3f(shear);
        child->inv_parent_U_mat_     = Conversion::fromMat3f(p_adj_R_new.inverse());

        child->updateModelMatrix();        
    }
}

void GComponent::Model::setShaderProperty(MyShader& shader)
{
    shader.setMat4("model", getModelMatrix());
    shader.setVec3("color", glm::vec3(1.0f));
}



