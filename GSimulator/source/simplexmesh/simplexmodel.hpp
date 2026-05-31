#ifndef SIMPLEXMODEL_HPP
#define SIMPLEXMODEL_HPP

#include "model/model.h"
#include "render/rendering_datastructure.hpp"
#include "render/rhi/rhi_device.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace GComponent{

class MyShader;

const glm::vec3 kBlue  = vec3(0.0f, 0.0f, 1.0f);
const glm::vec3 kRed   = vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 kGreen = vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 kPurple= vec3(1.0f, 0.0f, 1.0f);
const glm::vec3 kYellow= vec3(1.0f, 1.0f, 0.0f);
const glm::vec3 kCyan  = vec3(0.0f, 1.0f, 1.0f);
const glm::vec3 kWhite = vec3(1.0f, 1.0f, 1.0f);

class SimplexModel: public GComponent::Model
{
/// 数据域 Fields
protected:
    /* 资源管理项 */
    RhiMeshHandle mesh_;

    shared_ptr<IRhiDevice> rhi_device_;

    /* 初始化标志位 */
    bool isInit = false;

/// 纯虚函数 Pure Virtual Functions
public:
    /* 构造和析构 */
    SimplexModel() = default;
    virtual
    ~SimplexModel() = 0 { ClearGLScreenBuffer();};

    /* 删除拷贝构造与赋值函数 */
    SimplexModel(const SimplexModel &) = delete;
    SimplexModel &
    operator=(const SimplexModel &) = delete;

    /* 保留移动构造与赋值函数 */
    SimplexModel(SimplexModel && other){
        rhi_device_ = other.rhi_device_;

        mesh_ = other.mesh_;

        isInit = other.isInit;

        other.mesh_ = {};
        other.isInit = false;
        other.rhi_device_ = nullptr;
    }
    SimplexModel &
    operator=(SimplexModel && other){
        ClearGLScreenBuffer();
        rhi_device_ = other.rhi_device_;

        mesh_ = other.mesh_;

        isInit = other.isInit;

        other.mesh_ = {};
        other.isInit = false;
        other.rhi_device_ = nullptr;

        return *this;
    }

    /* 绘图函数 */
    virtual void
    Draw(MyShader *shader = nullptr) = 0;
protected:
    /* GL 资源初始化函数 */
    virtual void
    GLBufferInitialize() = 0;

/// 成员函数 Member Functions
public:
    void
    SetRhiDevice(const shared_ptr<IRhiDevice>& rhi_device)
    {
        rhi_device_ = rhi_device;
        ClearGLScreenBuffer();
        GLBufferInitialize();
        isInit = true;
    }

    void
    ClearGLScreenBuffer()
    {
        if(isInit)
        {
        rhi_device_->DestroyMesh(mesh_);
        mesh_ = {};
        isInit = false;
        }
    }


};

}

#endif // SIMPLEXMODEL_HPP
