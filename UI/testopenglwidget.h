#ifndef TESTOPENGLWIDGET_H
#define TESTOPENGLWIDGET_H

#include <QtCore/QTimer>
#include <QtOpenGL/QOpenGLFunctions_4_2_Core>
#include <QtOpenGLWidgets/QOpenGLWidget>

class QOpenGLShaderProgram;

class TestOpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit TestOpenGLWidget(QWidget *parent = nullptr);
    ~TestOpenGLWidget();
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
private:
    unsigned VAO, VBO;
    QTimer * m_timer;
    QOpenGLShaderProgram * m_program;
    QOpenGLFunctions_4_2_Core fCore;
signals:

};

#endif // TESTOPENGLWIDGET_H
