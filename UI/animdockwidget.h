#ifndef ANIMDOCKWIDGET_H
#define ANIMDOCKWIDGET_H

#include <QDockWidget>
#include <memory>
namespace Ui {
class AnimDockWidget;
}

class AnimDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit AnimDockWidget(QWidget *parent = nullptr);
    ~AnimDockWidget();
private slots:
    void on_pushButton_clicked();

private:
    Ui::AnimDockWidget *ui;
    std::unique_ptr<QOpenGLWidget> p_myWindow;
};

#endif // ANIMDOCKWIDGET_H
