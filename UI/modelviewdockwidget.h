#ifndef MODELVIEWDOCKWIDGET_H
#define MODELVIEWDOCKWIDGET_H

#include <QDockWidget>

namespace Ui {
class ModelViewDockWidget;
}

class QSplitter;

class ModelViewDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit ModelViewDockWidget(QWidget *parent = nullptr);
    ~ModelViewDockWidget();

protected:
    void resizeEvent(QResizeEvent * event) override;
private:
    Ui::ModelViewDockWidget *ui;
    QSplitter * m_spliter;
};

#endif // MODELVIEWDOCKWIDGET_H
