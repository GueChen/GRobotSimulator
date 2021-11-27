//#define _DEBUG_CONSOLE
#define MyGlad

#include <QCoreApplication>
#include <QOpenGLShaderProgram>
#include <glm/gtc/type_ptr.hpp>
#include <QMouseEvent>
#include <Component/Geometry/mesh.h>
#include <Component/Geometry/modelloader.h>
#include <QMimeData>
#include "ModelView.h"

using glm::vec3;
using std::vector;
using namespace GComponent;


ModelView::ModelView(QWidget *parent) :
    QOpenGLWidget(parent),
    gl(std::make_shared<MyGL>())
{

    m_timer = new QTimer(this);
    QObject::connect(m_timer, &QTimer::timeout, [this](){this->update();});
    m_timer->start(30);
    qDebug()<<"Constructor Over";
    this->setAcceptDrops(true);

}

ModelView::~ModelView()
{
    makeCurrent();
    cleanup();
    gl->glDeleteBuffers(1, &VBO);
    gl->glDeleteBuffers(1, &EBO);
    gl->glDeleteVertexArrays(1, &VAO);
    m_timer->stop();
    qDebug() << "Destructor Over";
    doneCurrent();
}

void ModelView::cleanup()
{
    delete m_program;
    delete phoneProgram;
    m_program = nullptr;
    phoneProgram = nullptr;
}
// TODO: 写初始化
void ModelView::initializeGL()
{


    gl->initializeOpenGLFunctions();

    if(!GLInit)
    {
        auto && [Vs, Is] = ModelLoader::readPlyFile("./Object/bunny/reconstruction/bun_zipper.ply");
        mesh = new Mesh(Vs, Is, {});
    }


    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex,   PathVert(Base));
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, PathFrag(Base));

    phoneProgram = new QOpenGLShaderProgram(this);
    phoneProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,   PathVert(Phone));
    phoneProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, PathFrag(Phone));

    m_program->link();
    phoneProgram->link();

    m_program->bind();
    phoneProgram->bind();

    mesh->setGL(gl);
    initializeGeometry(40, 0.05f);

    GLInit = true;
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ModelView::cleanup);
}

void ModelView::resizeGL(int w, int h)
{

    glViewport(0, 0, w, h);
}

// TODO: 重写绘制函数
void ModelView::paintGL()
{
    QOpenGLShaderProgram* pShader;
    makeCurrent();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    if(!black)
        pShader = m_program;
    else
        pShader = phoneProgram;

    pShader->bind();
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // 传递 Uniform 变量
    mat4 view = myCamera.GetViewMatrix();
    mat4 projection = glm::perspective(
                radians(myCamera.Zoom),
                (float)width() / height(),
                0.1f,
                1000.0f);

#ifdef _DEBUG_CONSOLE
    CheckPoint(view, projection);
#endif

    gl->setMatrices(uMBO, projection, view);

    mat4 model = glm::identity<mat4>();
    gl->setMat4(pShader->programId(), "model", model);
    gl->glBindVertexArray(VAO);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    gl->glDrawElements(GL_LINES, 160, GL_UNSIGNED_INT, 0);
    model = glm::scale(model, vec3(4.0f));
    gl->setMat4(pShader->programId(), "model", model);
    gl->setVec3(pShader->programId(), "viewPos", myCamera.Position);
    gl->setVec3(pShader->programId(), "light.dir", vec3(0.0f, -1.0f, 0.0f));
    gl->setVec3(pShader->programId(), "light.color", vec3(1.0f, 1.0f, 1.0f));

    mesh->Draw();

    pShader->release();


}

void ModelView::initializeGeometry(int num, float gSize)
{

    auto locs = genPos(num, gSize);
    auto lines = genTie(num);

    std::tie(VAO, VBO) = gl->genVABO(&locs[0], sizeof(vec3) * locs.size());
    gl->EnableVertexAttribArrays_continus(locs);
    EBO = gl->genEBO(lines);
    uMBO = gl->genMatrices();

}

vector<vec3> ModelView::genPos(int num, float gSize)
{
    vector<vec3> temp;
    temp.assign(num * 4, vec3{});

    {
        auto p = temp.begin();
        const float len = gSize * (num - 1);
        const float base = -len * 0.5f;
        /* 添加横对列 */
        for(int i = 0; i < num; ++i)                //  示意说明: x 添加 o 不添加
        {                                           //  👇  👉 X loc
            auto xPos = base + i * gSize;           //  Z   x x x x
            (*p++) = vec3(xPos, 0.0,     base);     //  loc o o o o
            (*p++) = vec3(xPos, 0.0, base+len);     //      o o o o
        }                                           //      x x x x
        /* 添加纵对行 */
        for(int i = 0; i < num; ++i)
        {
            auto zPos = base + i * gSize;
            (*p++) = vec3(base,       0.0f, zPos);
            (*p++) = vec3(base + len, 0.0f, zPos);
        }
    }
    return temp;
}

vector<Line> ModelView::genTie(int num)
{
    vector<Line> temp;
    temp.assign(num * 2, Line{});
    {
        int idx = 0;
        for(auto p = temp.begin();p != temp.end(); ++p, idx += 2)
        {
            (*p) = {idx, idx+1};
        }
    }
    return temp;
}

void ModelView::CheckPoint(mat4 view, mat4 pro)
{
    mat4 MVP = pro * view;
    {
        int count = 0;
        for(auto it = debugP.begin(); it != debugP.end(); it+= 2)
        {
            qDebug() << "COUNT " << count++;
            auto pp1 = MVP * glm::vec4(*it, 1.0f);
            auto pp2 = MVP * glm::vec4(*(it + 1), 1.0f);
            pp1 = pp1 / pp1.w;
            pp2 = pp2 / pp2.w;
            qDebug() << "Original:";
            qDebug("pp1 = (%.2f, %.2f, %.2f)", (*it).x, (*it).y, (*it).z);
            qDebug("pp2 = (%.2f, %.2f, %.2f)", (*(it+1)).x, (*(it+1)).y, (*(it+1)).z);
            qDebug() << "After MVP: ";
            qDebug("pp1 = (%.2f, %.2f, %.2f)", pp1.x, pp1.y, pp1.z);
            qDebug("pp2 = (%.2f, %.2f, %.2f)", pp2.x, pp2.y, pp2.z);
            qDebug() << "-----------------------------------------";
        }
    }

}

/**
 *
 * Event System Definition
 * 事件系统定义
 *
 * */
void ModelView::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta();

    if (!numDegrees.isNull()) {

        qDebug() << "Degree: " << numDegrees.x() << "," << numDegrees.y();
        myCamera.Move(0.0, 0.0f, numDegrees.y()/500.0f);
    }

    event->accept();

}

void ModelView::mousePressEvent(QMouseEvent *event)
{
    lastPos = event->globalPosition();
    if(event->button()== Qt::LeftButton)
    {
        setMouseTracking(true);
        FORWARD = true;
        qDebug() << "Left Pressed, Forward On";
    }
    if(event->button() == Qt::RightButton)
    {
        setMouseTracking(true);
        ROTATION = true;
        qDebug() << "Right Pressed, Rotation On";
    }
    if(event->button() == Qt::MiddleButton)
    {
        setMouseTracking(true);
        MOVE = true;
        qDebug() << "Middle Pressed, Move On";
    }

}

void ModelView::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()== Qt::LeftButton)
    {
        setMouseTracking(false);
        FORWARD = false;
        qDebug() << "Left Release, Forward Off";
    }
    if(event->button() == Qt::RightButton)
    {
        setMouseTracking(false);
        ROTATION = false;
        qDebug() << "Right Release, Rotation Off";
    }
    if(event->button() == Qt::MiddleButton)
    {
        setMouseTracking(false);
        MOVE = false;
        qDebug() << "Middle Release, Move Off";
    }
    lastPos = event->globalPosition();
}

void ModelView::mouseMoveEvent(QMouseEvent *event)
{

    auto curPos = event->globalPosition();
    auto delta =0.015f * (curPos - lastPos);
    if(MOVE)
    {
        myCamera.Move(-delta.x(), delta.y(), 0.0f); // += vec3(delta.x(), delta.y(), 0.0);
    }
    if(FORWARD)
    {
        myCamera.Move(0.0, 0.0, delta.y()); // myCamera.Position += vec3(0.0, 0.0, delta.y());
    }
    if(ROTATION)
    {
        delta *= 10.0f;
        myCamera.Rotation(delta.x(), delta.y());
    }
    lastPos = curPos;
}

void ModelView::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void ModelView::dropEvent(QDropEvent *event)
{
    QString name = event->mimeData()->urls().first().toLocalFile();
    qDebug("The File:\n%s\nDrag into the Area", name.toStdString().c_str());

    auto && [Vs, Is] = ModelLoader::readFile(name.toStdString());
    m_timer->stop();
    makeCurrent();
    mesh->~Mesh();
    mesh = new Mesh(Vs, Is, {});
    mesh->setGL(gl);
    m_timer->start();
}

