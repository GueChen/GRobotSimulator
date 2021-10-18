
#include <QOpenGLShaderProgram>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include "testopenglwidget.h"
#include <Component/GLTooler.hpp>
#include <QDebug>

static const float posData[] = {
    0.0f,   0.5f, 0.0f,
    0.66f,  -0.35f, 0.0f,
    -0.66f, -0.35f,  0.0f,
};

static const float color[] = {
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
};

TestOpenGLWidget::TestOpenGLWidget(QWidget *parent) :
    QOpenGLWidget(parent)
{
    m_timer = new QTimer(this);
    QObject::connect(m_timer, &QTimer::timeout, [this](){this->update();});
    QObject::connect(this, &QObject::destroyed, m_timer, &QTimer::deleteLater);
    m_timer->start(10);
    qDebug()<< "Test Widget Construct!";
}

TestOpenGLWidget::~TestOpenGLWidget()
{
    makeCurrent();
    fCore.glDeleteBuffers(1, &VBO);
    fCore.glDeleteVertexArrays(1, &VAO);
    m_timer->stop();
    qDebug() << "Test Widget Destruct!";
}

void TestOpenGLWidget::initializeGL()
{

    makeCurrent();
    fCore.initializeOpenGLFunctions();
#ifndef MyGlad
    m_program = new QOpenGLShaderProgram(this);
    m_program->addCacheableShaderFromSourceFile(
                QOpenGLShader::Vertex, PathVert(Triangle));
    m_program->addCacheableShaderFromSourceFile(
                QOpenGLShader::Fragment, PathFrag(Triangle));
    m_program->link();
    m_program->bind();

#endif

    fCore.glGenVertexArrays(1, &VAO);
    fCore.glBindVertexArray(VAO);

    fCore.glGenBuffers(1, &VBO);
    fCore.glBindBuffer(GL_ARRAY_BUFFER, VBO);
    fCore.glBufferData(GL_ARRAY_BUFFER, sizeof(posData) + sizeof(color), nullptr, GL_STATIC_DRAW);
    fCore.glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(posData), posData);
    fCore.glBufferSubData(GL_ARRAY_BUFFER, sizeof(posData), sizeof(color), color);

    fCore.glEnableVertexAttribArray(0);
    fCore.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);
    fCore.glEnableVertexAttribArray(1);
    fCore.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float),(void*)sizeof(posData));
}

void TestOpenGLWidget::resizeGL(int w, int h)
{
    fCore.glViewport(0.0f, 0.0f, w, h);

}

void TestOpenGLWidget::paintGL()
{

    fCore.glEnable(GL_DEPTH_TEST);
    fCore.glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    fCore.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    m_program->bind();
    fCore.glBindVertexArray(VAO);
    fCore.glDrawArrays(GL_TRIANGLES, 0, 3);

}
