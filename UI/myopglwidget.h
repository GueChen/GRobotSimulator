#ifndef MYOPGLWIDGET_H
#define MYOPGLWIDGET_H
#define MyGlad
#include <glm/glm.hpp>
#include <memory>
#include <QWindow>
#include <QTimer>
#include <QtOpenGLWidgets/QOpenGLWidget>

#ifndef MyGlad
#include <QOpenGLFunctions_4_2_Core>
#else
#include "Component/GLTooler.hpp"
#endif
#include "Component/Camera.hpp"
#include "Component/GStruct.hpp"

class QOpenGLShaderProgram;
class Camera;
class myOpglWidget : public QOpenGLWidget
#ifndef MyGlad
        , public QOpenGLFunctions_4_2_Core
#endif
{
    Q_OBJECT
public:
    explicit myOpglWidget(QWidget *parent = nullptr);
    ~myOpglWidget();
    bool black = true;
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void initializeGeometry(int num, float gSize = 0.5f);

private:
    QTimer * m_timer;

    unsigned VAO, VBO, EBO, uMBO;

#ifdef MyGlad
    std::shared_ptr<GComponent::MyGL> gl;
#endif

    QOpenGLShaderProgram * m_program = nullptr;

    std::vector<glm::vec3> debugP;

    GComponent::Camera myCamera;
    bool MOVE = false;
    bool FORWARD = false;
    bool ROTATION = false;
    QPointF lastPos;

    std::vector<glm::vec3> genPos(int num, float gSize);
    std::vector<GComponent::Line> genTie(int num);

    void CheckPoint(glm::mat4 view, glm::mat4 pro);
    void mouseMoveEvent(QMouseEvent * evnet) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

#endif // MYOPGLWIDGET_H
