#ifndef MODELVIEWWIDGET_H
#define MODELVIEWWIDGET_H

#include <QOpenGLWidget>

namespace GComponent {
    class MyGL;
}

class ModelViewWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit ModelViewWidget(QWidget *parent = nullptr);

signals:

};

#endif // MODELVIEWWIDGET_H
