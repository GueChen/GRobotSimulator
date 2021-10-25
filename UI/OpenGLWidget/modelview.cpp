#include <QTimer>
#include <QMouseEvent>
#include <QDropEvent>
#include <QMimeData>
#include "modelview.h"
#include "Component/Camera.hpp"
#include "Component/MyGL.hpp"
#include "Component/myshader.h"
#include "Component/Object/basegrid.h"
#include "Component/Geometry/modelloader.h"

using namespace GComponent;


ModelView::ModelView(QWidget *parent) :
    QOpenGLWidget(parent),
    gl(std::make_shared<MyGL>()),
    camera(std::make_unique<Camera>()),
    timer(std::make_unique<QTimer>(this)),
    grid(std::make_unique<BaseGrid>(20, 0.05))
{
    connect(timer.get(), &QTimer::timeout,  [this](){this->update();});
    timer->start(20);
    this->setAcceptDrops(true);
}

//TODO: 补充以下函数
void ModelView::initializeGL()
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
    qDebug() << "curShader: "<< curShader->programId();
    curShader->use();
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ModelView::cleanup);
}

void ModelView::paintGL()
{
    gl->glEnable( GL_DEPTH_TEST);
    gl->glClearColor( 0.2f, 0.2f, 0.2f, 1.0f);
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
    curShader->setMat4("model", model);
    curShader->setVec3("color", vec3(1.0f, 1.0f, 1.0f));
    curShader->setVec3("viewPos", camera->Position);
    curShader->setVec3("light.dir", vec3(0.0f, -1.0f, 0.0f));
    curShader->setVec3("light.color", vec3(0.8f, 1.0f, 0.9f));

    for(auto & [name, model]: mTable)
    {
        model->Draw();
    }
    curShader->release();
}

void ModelView::resizeGL(int w, int h)
{
    gl->glViewport(0, 0, width(), height());
}

void ModelView::cleanup()
{
    shaderMap.clear();
}

void ModelView::deleteMesh(const string &name)
{
    if(mTable.count(name))
    {
        mTable.erase(mTable.find(name));
    }
}

void ModelView::addMesh(const string & resource)
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

void ModelView::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta();

    if (!numDegrees.isNull()) {
        camera->Move(0.0, 0.0f, numDegrees.y()/500.0f);
    }

    event->accept();

}

void ModelView::mousePressEvent(QMouseEvent *event)
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

void ModelView::mouseReleaseEvent(QMouseEvent *event)
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

void ModelView::mouseMoveEvent(QMouseEvent *event)
{

    auto curPos = event->globalPosition();
    auto delta =0.015f * (curPos - lastMousePos);
    if(MiddleClick)
    {
        camera->Move(delta.x(), delta.y(), 0.0f);
    }
    if(LeftClick)
    {
        camera->Move(0.0, 0.0, delta.y());
    }
    if(RightClick)
    {
        delta *= 10.0f;
        camera->Rotation(delta.x(), delta.y());
    }
    lastMousePos = curPos;
}

void ModelView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void ModelView::dropEvent(QDropEvent *event)
{
    QString filePath = event->mimeData()->urls().first().toLocalFile();
    try {
        addMesh(filePath.toStdString());
    }  catch (_exception e) {
        qDebug() << "模型无法加载";
    }
}

