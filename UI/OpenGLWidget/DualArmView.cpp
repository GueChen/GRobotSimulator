#include <QTimer>
#include <QMouseEvent>
#include <QDropEvent>
#include <QMimeData>

#include "DualArmView.h"
#include "Component/Camera.hpp"
#include "Component/MyGL.hpp"
#include "Component/myshader.h"
#include "Component/Object/basegrid.h"
#include "Component/Geometry/modelloader.h"
#include "Component/Object/Robot/dual_arm_platform.h"

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

//TODO: 补充以下函数
void DualArmView::initializeGL()
{
    gl->initializeOpenGLFunctions();
    uMBO = gl->genMatrices();
    makeCurrent();
    grid = std::make_unique<BaseGrid>(20, 0.05);
    grid->setGL(gl);
    shaderMap.insert(genShaderMap(Base));
    shaderMap.insert(genShaderMap(Phone));
    shaderMap.insert(genShaderMap(Color));

    for(auto &[key, shader]:shaderMap)
    {
        shader->setGL(gl);
        shader->link();
    }
    curShader = shaderMap["Base"].get();

    curShader->use();

    makeCurrent();

    DUAL_ARM_PLATFORM::setGL(gl);

    for(auto &[name, pmesh] : mTable)
    {
        pmesh->setGL(gl);
    }

    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &DualArmView::cleanup);

}

void DualArmView::paintGL()
{
    gl->glEnable( GL_DEPTH_TEST);
    gl->glClearColor( 0.1f, 0.13f, 0.15f, 1.0f);
    gl->glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    curShader = shaderMap["Base"].get();
    curShader->use();
    mat4 view = camera->GetViewMatrix();
    mat4 projection = glm::perspective(radians(camera->Zoom),
                                       (float)width() / height(),
                                       0.1f,
                                       1000.0f);
    gl->setMatrices(uMBO, projection, view);
    mat4 model = glm::identity<mat4>();

    curShader->setMat4("model", model);
    makeCurrent();
    grid->Draw();

    curShader->release();
    curShader = shaderMap["Color"].get();
    curShader->use();

    curShader->setVec3("viewPos", camera->Position);

    vec3 lightDir = vec3(-0.5f, -1.0f, -1.0f);
    lightDir = glm::mat3(glm::rotate(model, glm::radians(camera->Yaw + 90.0f), vec3(0.0f, -1.0f, 0.0f)))
            * lightDir;
    curShader->setVec3("light.dir", lightDir);
    curShader->setVec3("light.color", vec3(1.0f, 1.0f, 1.0f));

    robot->setLeftColor(vec3(0.75f, 0.55f, 0.70f));
    robot->setRightColor(vec3(0.55f, 0.75f, 0.70f));

    curShader->setVec3("color", vec3(0.85f, 0.85f, 0.75f));
    robot->Draw(curShader);
    curShader->release();
}

void DualArmView::resizeGL(int w, int h)
{
    gl->glViewport(0, 0, width(), height());
}

void DualArmView::cleanup()
{
    shaderMap.clear();
}

void DualArmView::deleteMesh(const string &name)
{
    if(mTable.count(name))
    {
        mTable.erase(mTable.find(name));
    }
}

void DualArmView::addMesh(const string & resource)
{
    auto begin  =   resource.find_last_of('/'),
         end    =   resource.find_last_of('.');
    auto name   =   resource.substr(begin + 1, end - begin);
    if(mTable.count(name))
    {
        qDebug() << "模型已存在";
        return;
    }

    auto && [Vs, Is] = ModelLoader::readFile(resource);

    timer->stop();

    makeCurrent();
    auto p_TemplateMesh = std::make_unique<Mesh>(Vs, Is, std::vector<Texture>{});
    p_TemplateMesh->setGL(gl);
    mTable.insert(std::make_pair(name, std::move(p_TemplateMesh)));
    timer->start(20);
}

/**********************************************/
/**
 *          Widget Event Method
 *              窗体事件函数
 * ******************************************/

void DualArmView::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta();

    if (!numDegrees.isNull()) {
        camera->Move(0.0, 0.0f, numDegrees.y()/500.0f);
    }

    event->accept();

}

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

void DualArmView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void DualArmView::dropEvent(QDropEvent *event)
{
    QString filePath = event->mimeData()->urls().first().toLocalFile();
    try {
        addMesh(filePath.toStdString());
    }catch (_exception e) {
        qDebug() << "模型无法加载";
    }
}

