#ifndef SIMPLEXMODEL_HPP
#define SIMPLEXMODEL_HPP

#include "model/model.h"
#include "render/rendering_datastructure.hpp"
#include "render/mygl.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace GComponent{

class MyGL;
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
    unsigned VAO = 0,
             VBO = 0,
             EBO = 0;

    /* GL指针 */
    shared_ptr<MyGL> gl;

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
        gl = other.gl;

        VAO = other.VAO;
        VBO = other.VBO;
        EBO = other.EBO;

        isInit = other.isInit;

        other.VAO = other.VBO = other.EBO = 0;
        other.isInit = false;
        other.gl     = nullptr;
    }
    SimplexModel &
    operator=(SimplexModel && other){
        gl = other.gl;

        VAO = other.VAO;
        VBO = other.VBO;
        EBO = other.EBO;

        isInit = other.isInit;

        other.VAO = other.VBO = other.EBO = 0;
        other.isInit = false;
        other.gl     = nullptr;

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
    /* GL 资源管理函数 */
    void
    SetGL(const shared_ptr<MyGL> & other)
    {
        gl = other;
        ClearGLScreenBuffer();
        GLBufferInitialize();
        isInit = true;
    }

    void
    ClearGLScreenBuffer()
    {
        if(isInit)
        {
        gl->glDeleteBuffers(1, &VBO);
        gl->glDeleteBuffers(1, &EBO);
        gl->glDeleteVertexArrays(1, &VAO);

        VAO = VBO = EBO = 0;
        isInit = false;
        }
    }


};

}

#endif // SIMPLEXMODEL_HPP
