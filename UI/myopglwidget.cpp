//#define _DEBUG_CONSOLE
#define MyGlad

#include <QCoreApplication>
#include <QOpenGLShaderProgram>
#include <glm/gtc/type_ptr.hpp>
#include <QMouseEvent>
#include <Component/Geometry/mesh.h>
#include "myopglwidget.h"

using glm::vec3;
using std::vector;
using namespace GComponent;


myOpglWidget::myOpglWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    gl(std::make_shared<MyGL>())
{

    m_timer = new QTimer(this);
    QObject::connect(m_timer, &QTimer::timeout, [this](){this->update();});
    m_timer->start(30);
    qDebug()<<"Constructor Over";

}

myOpglWidget::~myOpglWidget()
{
    makeCurrent();
    gl->glDeleteProgram(m_program->programId());
    gl->glDeleteBuffers(1, &VBO);
    gl->glDeleteBuffers(1, &EBO);
    gl->glDeleteVertexArrays(1, &VAO);
    m_timer->stop();
    qDebug() << "Destructor Over";
}
// TODO: 写初始化
void myOpglWidget::initializeGL()
{
#ifdef MyGlad
    gl->initializeOpenGLFunctions();
#else
    initializeOpenGLFunctions();
#endif

    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceFile(
                QOpenGLShader::Vertex, PathVert(Base));
    m_program->addShaderFromSourceFile(
                QOpenGLShader::Fragment, PathFrag(Base));

    m_program->link();
    m_program->bind();

    qDebug() << "Shader Constructor Over!";
    initializeGeometry(10, 0.5f);
    qDebug() << "Initialize Over";
}

// TODO: 重写调尺寸
void myOpglWidget::resizeGL(int w, int h)
{

    glViewport(0, 0, w, h);
}

// TODO: 重写绘制函数
void myOpglWidget::paintGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    Mesh m({{glm::vec3(0.0f),glm::vec3{},glm::vec2{}}},{0},{});
    m.setGL(gl);
    if(black)
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    else
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // 绑定 Shader

    m_program->bind();
    gl->setBool(m_program->programId(), "colorReverse", !black);

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

#ifdef MyGlad
    gl->setMatrices(uMBO, projection, view);
#else
    glBindBuffer(GL_UNIFORM_BUFFER, uMBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,            sizeof(mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), glm::value_ptr(view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
#endif

    mat4 model = glm::identity<mat4>();
    gl->setMat4(m_program->programId(), "model", model);

    gl->glBindVertexArray(VAO);
    gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // gl.glDrawElements(GL_LINES, 20, GL_UNSIGNED_INT, 0);
    gl->glDrawArrays(GL_LINES, 0, 40);

    m_program->release();


}

void myOpglWidget::initializeGeometry(int num, float gSize)
{

    auto locs = genPos(num, gSize);
    auto lines = genTie(num);
    qDebug() << "Begin Gen VABO: locs size = " << locs.size();
#ifdef MyGlad
    std::tie(VAO, VBO) = gl->genVABO(&locs[0], sizeof(vec3) * locs.size());
    gl->EnableVertexAttribArrays_continus(locs);
    EBO = gl->genEBO(genTie(num));
    uMBO = gl->genMatrices();
#else
    // 等价于之前的 GenVABO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)* locs.size(), &locs[0], GL_STATIC_DRAW);

    // 等价于之前的 EnableVertexAttribArrays
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                             reinterpret_cast<void * >(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                             reinterpret_cast<void * >(2 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // 等价于之前的 genEBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Line) * lines.size(), &lines[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // 等价于 genMatrices
    glGenBuffers(1, &uMBO);
    glBindBuffer(GL_UNIFORM_BUFFER, uMBO);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uMBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

#endif
#ifdef _DEBUG_CONSOLE
    for(auto it = locs.begin(); it != locs.end(); ++it)
    {
        debugP.push_back(*it);
    }
#endif
}

vector<vec3> myOpglWidget::genPos(int num, float gSize)
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

vector<Line> myOpglWidget::genTie(int num)
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

void myOpglWidget::CheckPoint(mat4 view, mat4 pro)
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

void myOpglWidget::mousePressEvent(QMouseEvent *event)
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

void myOpglWidget::mouseReleaseEvent(QMouseEvent *event)
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

void myOpglWidget::mouseMoveEvent(QMouseEvent *event)
{

    auto curPos = event->globalPosition();
    auto delta =0.015f * (curPos - lastPos);
    if(MOVE)
    {
        myCamera.Position += vec3(delta.x(), delta.y(), 0.0);
    }
    if(FORWARD)
    {

        myCamera.Position += vec3(0.0, 0.0, delta.y());
    }
    if(ROTATION)
    {
        delta *= 5.0f;
        myCamera.Rotation(delta.x(), delta.y());
    }
    lastPos = curPos;
}
