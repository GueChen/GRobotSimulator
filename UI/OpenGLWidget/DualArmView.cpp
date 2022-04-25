#include "DualArmView.h"

#include "render/Camera.hpp"
#include "render/myshader.h"
#include "render/MyGL.hpp"
#include "simplexmesh/GBasicMesh"
#include "simplexmesh/SimplexModel.hpp"
#include "model/basegrid.h"

#include "function/modelloader.h"
#include "model/robot/dual_arm_platform.h"

#include <QtCore/QTimer>
#include <QtCore/QMimeData>
#include <QtGUI/QMouseEvent>
#include <QtGUI/QDropEvent>

using std::make_pair;
using std::make_unique;
using std::make_shared;

using namespace GComponent;


DualArmView::DualArmView(QWidget *parent) :
    QOpenGLWidget(parent),
    gl(make_shared<MyGL>()),
    camera(make_unique<Camera>()),
    timer(make_unique<QTimer>(this)),
    grid(make_unique<BaseGrid>(20, 0.05)),
    robot(make_unique<DUAL_ARM_PLATFORM>())
{
    connect(timer.get(), &QTimer::timeout,  [this](){this->update();});
    timer->start(20);
    this->setAcceptDrops(true);
}

JointsPair DualArmView::getJoints()
{
    return robot->getJoints();
}

KUKA_IIWA_MODEL* DualArmView::getLeftRobot() const
{
    return robot->getLeftRobot();
}

KUKA_IIWA_MODEL* DualArmView::getRightRobot() const
{
    return robot->getRightRobot();
}


//****************************************************************//

//TODO: 补充以下函数
void DualArmView::initializeGL()
{
    grid = std::make_unique<BaseGrid>(20, 0.05);

    shaderMap.insert(genShaderMap(Base));
    shaderMap.insert(genShaderMap(Phone));
    shaderMap.insert(genShaderMap(Color));
    shaderMap.insert(genShaderMap(LineColor));

    setGL();
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &DualArmView::cleanup);

}

void DualArmView::setGL()
{
    /* 初始化 GL 函数并获取 context */
    gl->initializeOpenGLFunctions();
    makeCurrent();

    /* 初始化内存矩阵 */
    uMBO = gl->genMatrices();

    /* 对着色器传递 GL 指针 */
    for(auto &[key, shader]:shaderMap)
    {
        shader->setGL(gl);
        shader->link();
    }

    /* 对网格资源传递 GL 指针 */
    for(auto &[name, pmesh] : meshTable)
    {
        pmesh->setGL(gl);
    }

    /* 对简单单纯形传递 GL 指针 */
    for(auto &[name, pModel] : modelTable)
    {
        pModel->setGL(gl);
    }

    /* 未归并统一管理的部分 Model */
    // FIXME: 重构时将该部分纳入统一管理
    grid->setGL(gl);
    DUAL_ARM_PLATFORM::setGL(gl);
}

void DualArmView::paintGL()
{
    using namespace glm;

    gl->glEnable( GL_DEPTH_TEST);
    gl->glClearColor( 0.1f, 0.13f, 0.15f, 1.0f);
    gl->glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    curShader = shaderMap["Base"].get();
    curShader->use();
    mat4 view       = camera->GetViewMatrix();
    mat4 projection = perspective(radians(camera->Zoom),
                                  (float)width() / height(),
                                  0.1f,
                                  1000.0f);
    gl->setMatrices(uMBO, projection, view);
    mat4 model      = identity<mat4>();
    curShader->setMat4("model", model);

    makeCurrent();
    grid->Draw();

    curShader->release();
    curShader = shaderMap["Color"].get();
    curShader->use();

    curShader->setVec3("viewPos", camera->Position);

    vec3 lightDir = vec3(-0.5f, -1.0f, -1.0f);
    lightDir      = mat3(rotate(model, radians(camera->Yaw + 90.0f), vec3(0.0f, -1.0f, 0.0f))) * lightDir;
    curShader->setVec3("light.dir", lightDir);
    curShader->setVec3("light.color", vec3(1.0f, 1.0f, 1.0f));

    robot->setLeftColor(vec3(0.75f, 0.55f, 0.70f));
    robot->setRightColor(vec3(0.55f, 0.75f, 0.70f));
    curShader->setBool("NormReverse", false);
    curShader->setVec3("color", vec3(0.85f, 0.85f, 0.75f));
    robot->Draw(curShader);

//    curShader->setVec3("color", vec3(0.55f, 0.45f, 0.85f));
    curShader->setBool("NormReverse", true);

    for(auto & [name, model]: modelTable)
    {
        if(name.find("Line") != -1){
            curShader = shaderMap["LineColor"].get();
            curShader->use();
            curShader->setMat4("model", glm::identity<mat4>());
        }

        model->Draw(curShader);

        if(name.find("Line") != -1){
            curShader = shaderMap["Color"].get();
            curShader->use();
        }
    }

    curShader->release();
}

/// 窗口大小回调
/// 在窗口大小发生改变时负责调整窗口大小
void DualArmView::resizeGL(int w, int h)
{
    gl->glViewport(0, 0, width(), height());
}


//****************************************************************//
/// <summary>
///  清理函数
/// </summary>
/// 负责在重初始化前清理资源 避免内存泄漏
void DualArmView::cleanup()
{
    shaderMap.clear();
    for(auto & [name, model]: modelTable)
    {
        model->ClearGL();
    }
}


/// <summary>
/// Mesh 资源删除函数
/// </summary>
void DualArmView::deleteMesh(const string &name)
{
    if(meshTable.count(name))
    {
        meshTable.erase(meshTable.find(name));
    }
}

/// <summary>
/// Mesh 资源添加函数
/// </summary>
void DualArmView::addMesh(const string & resource)
{
    auto begin  =   resource.find_last_of('/'),
         end    =   resource.find_last_of('.');
    auto name   =   resource.substr(begin + 1, end - begin);
    if(meshTable.count(name))
    {
        qDebug() << "模型已存在";
        return;
    }

    auto && [Vs, Is] = ModelLoader::readFile(resource);

    timer->stop();

    makeCurrent();
    unique_ptr<Mesh> p_TemplateMesh = make_unique<Mesh>(Vs, Is, std::vector<Texture>{});
    p_TemplateMesh->setGL(gl);
    meshTable.insert(make_pair(name, std::move(p_TemplateMesh)));
    timer->start(20);

}

template <class _Derived>
void DualArmView::addSimplexModel(const string &name, unique_ptr<_Derived>&& pModel)
{
    /* 编译时 类型检测 */
    static_assert (std::is_base_of<SimplexModel, _Derived>::value,
            "Can't cast _Derived Class From Type");

    /* 为新添加 Mesh 配置 GL Context */
    makeCurrent();
    pModel->setGL(gl);

    /* 检测是否有重名并删除 */
    deleteSimplexModel(name);

    /* 把新对象插入字典中 */
    modelTable.insert(make_pair(name, std::move(pModel)));
}

template void
DualArmView::addSimplexModel(const string&, unique_ptr<GBall>&&);
template void
DualArmView::addSimplexModel(const string&, unique_ptr<GCurves>&&);
template void
DualArmView::addSimplexModel(const string&, unique_ptr<GLine>&&);

void
DualArmView::deleteSimplexModel(const string& name)
{
    if(modelTable.count(name)){
       modelTable.erase(name);
    }
}

void
DualArmView::clearSimplexModel()
{
    modelTable.clear();
}
/**********************************************/
/**
 *          Widget Event Method
 *              窗体事件函数
 * ******************************************/

/// <summary>
/// 滚轮回调函数
/// </summary>
void DualArmView::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta();

    if (!numDegrees.isNull()) {
        camera->Move(0.0, 0.0f, numDegrees.y()/500.0f);
    }

    event->accept();

}

/// <summary>
/// 鼠标按下函数
/// </summary>
void DualArmView::mousePressEvent(QMouseEvent *event)
{
    lastMousePos = event->globalPosition();
    if(event->button()== Qt::LeftButton)
    {
        setMouseTracking(true);
        LeftClick = true;
    }
    if(event->button() == Qt::RightButton)
    {
        setMouseTracking(true);
        RightClick = true;
    }
    if(event->button() == Qt::MiddleButton)
    {
        setMouseTracking(true);
        MiddleClick = true;
    }

}

/// <summary>
/// 鼠标释放回调函数
/// </summary>
void DualArmView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()== Qt::LeftButton)
    {
        setMouseTracking(false);
        LeftClick = false;
    }
    if(event->button() == Qt::RightButton)
    {
        setMouseTracking(false);
        RightClick = false;
    }
    if(event->button() == Qt::MiddleButton)
    {
        setMouseTracking(false);
        MiddleClick = false;
    }
    lastMousePos = event->globalPosition();
}

/// <summary>
/// 鼠标移动回调回调函数
/// </summary>
void DualArmView::mouseMoveEvent(QMouseEvent *event)
{

    auto curPos = event->globalPosition();
    auto delta =0.015f * (curPos - lastMousePos);
    if(MiddleClick)
    {
        camera->Move(0.0, delta.y(), 0.0f);
    }
    if(LeftClick)
    {
        camera->Move(0.0, 0.0, delta.x());
    }
    if(RightClick)
    {
        delta *= 10.0f;
        camera->Rotation(delta.x(), delta.y());
    }
    lastMousePos = curPos;
}

/// <summary>
/// 接受拖拽回调函数
/// </summary>
void DualArmView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

/// <summary>
/// 拖拽释放回调函数
/// </summary>
void DualArmView::dropEvent(QDropEvent *event)
{
    QString filePath = event->mimeData()->urls().first().toLocalFile();
    try {
        addMesh(filePath.toStdString());
    }catch (_exception e) {
        qDebug() << "模型无法加载";
    }
}

