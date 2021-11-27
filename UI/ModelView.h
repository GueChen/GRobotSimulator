#ifndef MODELVIEW_H
#define MODELVIEW_H
#define MyGlad
#include <glm/glm.hpp>
#include <memory>
#include <QTimer>
#include <QtOpenGLWidgets/QOpenGLWidget>

#include "Component/MyGL.hpp"
#include "Component/Camera.hpp"
#include "Component/GStruct.hpp"

class QOpenGLShaderProgram;
class Camera;
namespace GComponent {
    class Mesh;
}

class ModelView : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit ModelView(QWidget *parent = nullptr);
    ~ModelView();
    bool black = true;
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void initializeGeometry(int num, float gSize = 0.5f);

private:
    QTimer * m_timer;
    unsigned VAO, VBO, EBO, uMBO;

    GComponent::Mesh * mesh;
#ifdef MyGlad
    std::shared_ptr<GComponent::MyGL> gl;
#endif
    QOpenGLShaderProgram * m_program = nullptr;
    QOpenGLShaderProgram * phoneProgram = nullptr;
    int count = 0;
    std::vector<glm::vec3> debugP;
    bool GLInit = false;

    GComponent::Camera myCamera;
    bool MOVE = false;
    bool FORWARD = false;
    bool ROTATION = false;
    QPointF lastPos;

    std::vector<glm::vec3> genPos(int num, float gSize);
    std::vector<GComponent::Line> genTie(int num);

    void CheckPoint(glm::mat4 view, glm::mat4 pro);
    void cleanup();

    void wheelEvent(QWheelEvent * event) override;
    void mouseMoveEvent(QMouseEvent * evnet) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent * event) override;
    void dropEvent(QDropEvent * evnt) override;
};

#endif // MODELVIEW_H
