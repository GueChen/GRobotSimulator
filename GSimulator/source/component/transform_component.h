/**
 *  @file  	transform_component.h
 *  @brief 	supply the transformation for model
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @data   xxx xx, 2022
 *  @update May 10, 2023 supply trans, rot, scale datas
 **/
#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include "Component/component.hpp"

#include "GComponent/gtransform.hpp"

#include <Eigen/Dense>

namespace GComponent {
using Vector3   = Eigen::Vector3f;
using Matrix4x4 = Eigen::Matrix4f;
using Matrix3x3 = Eigen::Matrix3f;

class TransformComponent : public Component {		
public:
    explicit TransformComponent(Model* ptr_parent) : Component(ptr_parent) {}
    TransformComponent(Model* ptr_parent, const Matrix4x4& mat);
    TransformComponent(Model* ptr_parent, const Vector3& pos, const Vector3& rot, const Vector3& scale);

	~TransformComponent() = default;
    [[deprecated("no implementation for glm axis getting")]]
    inline Matrix4x4 GetModelGlobal() const               { return parent_mat_ * model_mat_; }
    void             SetModelLocal(const Matrix4x4& mat, bool update = true);
    inline Matrix4x4 GetModelLocal()  const               { return model_mat_; }


    Matrix4x4        GetTranslationModelMatrix() const;
    Matrix4x4        GetModelMatrixWithoutScale()const;

    inline Matrix4x4 GetParentMatrix()           const    { return parent_mat_; }
    void             SetParentMatrix(const Matrix4x4& mat){ parent_mat_ = mat; }

	void             SetTransLocal(const Vector3& translation, bool updateflag = true);
    inline Vector3   GetTransLocal()  const               { return position_;}
    Vector3          GetTransGlobal() const;

    void             SetRotLocal(const Vector3& rotation, bool updateflag = true);
    inline Vector3   GetRotLocal()    const               { return rotation_;}
    Vector3          GetRotGlobal()   const;

    void             SetScale(const Vector3 scale, bool updateflag = true);
    inline Vector3   GetScale()       const               { return scale_;}
        
    inline  bool     GetIsDirty()     const               { return is_dirty_; }
    inline  void     SetIsDirty(bool dirty)               { is_dirty_ = dirty; }
    
    virtual const string_view& 
                     GetTypeName()    const override;
protected:
    void             UpdateModelMatrix();
    void             UpdateChildrenMatrix(const Matrix3x3& parent_scale_mat);

    void             tickImpl(float delta) override;

protected:
	Vector3   position_  = Vector3::Zero();
	Vector3   rotation_  = Vector3::Zero();
	Vector3   scale_     = Vector3::Ones();
	Vector3   shear_	 = Vector3::Zero();	
                         
    Matrix4x4 parent_mat_   = Matrix4x4::Identity();
    Matrix4x4 model_mat_    = Matrix4x4::Identity();
    Matrix3x3 inv_parent_U_ = Matrix3x3::Identity();

    bool      is_dirty_ = false;

public:
    static constexpr const std::string_view type_name = "TransformComponent";
};

using TransformCom = TransformComponent;
}

#endif // !_TRANSFORM_H
