#ifndef MODELVIEWWIDGET_H
#define MODELVIEWWIDGET_H

#include <QWidget>

namespace Ui {
class ModelViewWidget;
}

class ModelViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModelViewWidget(QWidget *parent = nullptr);
    ~ModelViewWidget();

private:
    Ui::ModelViewWidget *ui;
};

#endif // MODELVIEWWIDGET_H
