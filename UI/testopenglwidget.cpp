
#include "testopenglwidget.h"

#include "render/mygl.hpp"

#include <glm/glm.hpp>

#include <QtOpenGL/QOpenGLShaderProgram>
#include <QtGUI/QOpenGLContext>
#include <QtGUI/QOpenGLExtraFunctions>

#include <iostream>
#include <chrono>

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
    QOpenGLWidget(parent), gl(std::make_shared<GComponent::MyGL>())
{
    m_timer = new QTimer(this);
    GComponent::SceneManager::getInstance().RegisteredUIHandle("testwidget", this);
    /*QObject::connect(m_timer, &QTimer::timeout, [this](){this->update();});
    QObject::connect(this, &QObject::destroyed, m_timer, &QTimer::deleteLater);
    m_timer->start(20);*/
}

TestOpenGLWidget::~TestOpenGLWidget()
{
    makeCurrent();
    
    gl->glDeleteBuffers(1, &VBO);
    gl->glDeleteVertexArrays(1, &VAO);
    //glDeleteBuffers(1, &VBO);
    //glDeleteVertexArrays(1, &VAO);

   // m_timer->stop();
  
}

void TestOpenGLWidget::initializeGL()
{
    gl->initializeOpenGLFunctions();
    makeCurrent();
#ifndef MyGlad
    m_program = new QOpenGLShaderProgram(this);
    m_program->addCacheableShaderFromSourceFile(
                QOpenGLShader::Vertex, PathVert(Triangle));
    m_program->addCacheableShaderFromSourceFile(
                QOpenGLShader::Fragment, PathFrag(Triangle));
    m_program->link();
    m_program->bind();

#endif
    shader = new GComponent::MyShader(this, PathVert(Triangle), PathFrag(Triangle));
    shader->setGL(gl);
    shader->link();

    std::tie(VAO, VBO) = gl->genVABO(nullptr, sizeof(posData) + sizeof(color));
     
    gl->glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(posData), posData);
    gl->glBufferSubData(GL_ARRAY_BUFFER, sizeof(posData), sizeof(color), color);

    gl->glEnableVertexAttribArray(0);
    gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3* sizeof(float), (void*)0);
    gl->glEnableVertexAttribArray(1);
    gl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float),(void*)sizeof(posData));
}

void TestOpenGLWidget::resizeGL(int w, int h)
{
    gl->glViewport(0.0f, 0.0f, w, h);

}

void TestOpenGLWidget::paintGL()
{
    static std::chrono::time_point  last_point = std::chrono::steady_clock::now();
    static float rotation = 0.0f;
    gl->glEnable(GL_DEPTH_TEST);
    gl->glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    shader->use();
    glm::mat4 model(1.0f);
    model = glm::rotate(model, rotation, glm::vec3(0.0, 0.0, 1.0f));
    shader->setMat4("model", model);

    //gl->glBindVertexArray(VAO);
    //gl->glDrawArrays(GL_TRIANGLES, 0, 3);
    rotation += 0.1f;
    if (rotation > 3.14f)
        rotation = -3.2f;
    std::chrono::time_point now = std::chrono::steady_clock::now();
    //std::cout << "the rotation angle: " << rotation << std::endl;
    std::cout << "triangle span     :" << std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(now - last_point) << std::endl;
    last_point = now;
}
