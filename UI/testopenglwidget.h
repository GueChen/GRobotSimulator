#ifndef TESTOPENGLWIDGET_H
#define TESTOPENGLWIDGET_H

#include "render/mygl.hpp"
#include "render/myshader.h"
#include "render/camera.hpp"

#include "manager/scenemanager.h"

#include <QtCore/QTimer>
#include <QtOpenGLWidgets/QOpenGLWidget>

#include <unordered_map>

class QOpenGLShaderProgram;

class TestOpenGLWidget : public QOpenGLWidget
{
    friend class GComponent::SceneManager;
    Q_OBJECT
public:
    explicit TestOpenGLWidget(QWidget *parent = nullptr);
    ~TestOpenGLWidget();
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
private:
    std::shared_ptr<GComponent::MyGL> gl;
    GComponent::MyShader* shader;

    unsigned VAO, VBO;
    QTimer * m_timer;
    QOpenGLShaderProgram * m_program;
    
    //QOpenGLFunctions_4_2_Core fCore;
signals:

};

#endif // TESTOPENGLWIDGET_H
