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

#include "Component/Object/model.h"

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

    /************/
    initModel();
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
    curShader->setVec3("color", vec3(0.5f, 0.5f, 0.7f));
    curShader->setVec3("viewPos", camera->Position);
    curShader->setVec3("light.dir", vec3(-1.0f, -1.0f, -1.0f));
    curShader->setVec3("light.color", vec3(0.7f, 0.7f, 0.7f));

//    for(auto & [name, model]: mTable)
//    {
//        model->Draw();
//    }

    static float angle = 0.0f;
    angle+= 0.5f;
    models[1]->setAxis(vec3(0.0f, 1.0f, 0.0f));
    models[1]->setRotate(angle);
    models[2]->setAxis(vec3(0.0f, 0.0f, 1.0f));
    models[2]->setRotate(angle);

    for(auto & model: models)
    {
        mat4 m = glm::identity<mat4>();
        m = model->_parentMatrix * model->_matrixModel * m;
        curShader->setMat4("model", m);
        if(model->getMesh() != "")
            mTable[model->getMesh()]->Draw();
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

// TODO: HARD-CODE
void ModelView::initModel()
{
    static bool hasInit = false;
    if(!hasInit)
    {
    mTable.insert(std::make_pair(
                      "Base",
                      std::make_unique<Mesh>(
                          ModelLoader::getMesh(PathModel(iiwa14_base.STL)))));
    mTable.insert(std::make_pair(
                      "Link1",
                      std::make_unique<Mesh>(
                          ModelLoader::getMesh(PathModel(iiwa14_link_1.STL)))));
    mTable.insert(std::make_pair(
                      "Link2",
                      std::make_unique<Mesh>(
                          ModelLoader::getMesh(PathModel(iiwa14_link_2.STL)))));
    mTable.insert(std::make_pair(
                      "Link3",
                      std::make_unique<Mesh>(
                          ModelLoader::getMesh(PathModel(iiwa14_link_3.STL)))));
    mTable.insert(std::make_pair(
                      "Link4",
                      std::make_unique<Mesh>(
                          ModelLoader::getMesh(PathModel(iiwa14_link_4.STL)))));
    mTable.insert(std::make_pair(
                      "Link5",
                      std::make_unique<Mesh>(
                          ModelLoader::getMesh(PathModel(iiwa14_link_5.STL)))));
    mTable.insert(std::make_pair(
                      "Link6",
                      std::make_unique<Mesh>(
                          ModelLoader::getMesh(PathModel(iiwa14_link_6.STL)))));
    mTable.insert(std::make_pair(
                      "Flansch",
                      std::make_unique<Mesh>(
                          ModelLoader::getMesh(PathModel(flanschExten.STL)))));

    for(int i = 0; i < 8; ++i)
    {
        models.push_back(std::make_shared<Model>());
    }

    models[0]->setMesh("Base");
    models[1]->setMesh("Link1");
    models[2]->setMesh("Link2");
    models[3]->setMesh("Link3");
    models[4]->setMesh("Link4");
    models[5]->setMesh("Link5");
    models[6]->setMesh("Link6");
    models[7]->setMesh("Flansch");

    mat4 im = glm::identity<mat4>();
    models[6]->appendChild(models[7], glm::translate(im, vec3(0.0f, 0.0809f, 0.0607f)));
    models[5]->appendChild(models[6], glm::translate(im, vec3(0.0f, 0.2155f, -0.0607f)));
    models[4]->appendChild(models[5], glm::translate(im, vec3(0.0f, 0.1845f, 0.0f)));
    models[3]->appendChild(models[4], glm::translate(im, vec3(0.0f, 0.2155f, 0.0f)));
    models[2]->appendChild(models[3], glm::translate(im, vec3(0.0f, 0.2045f, 0.0f)));
    models[1]->appendChild(models[2], glm::translate(im, vec3(0.0f, 0.2025f, 0.0f)));
    models[0]->appendChild(models[1], glm::translate(im, vec3(0.0f, 0.1575f, 0.0f)));
    hasInit = true;
    }

    makeCurrent();
    for(auto &[name, pmesh] : mTable)
    {
        pmesh->setGL(gl);
    }

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

