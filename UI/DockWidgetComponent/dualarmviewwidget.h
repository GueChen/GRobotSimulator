#ifndef DUALARMVIEWWIDGET_H
#define DUALARMVIEWWIDGET_H

#include <QWidget>
#include <QVariant>

namespace Ui {
class DualArmViewWidget;
}

namespace GComponent{
    class Joint;
}

class DualArmViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DualArmViewWidget(QWidget *parent = nullptr);
    ~DualArmViewWidget();
    Q_INVOKABLE void setLeftArmRotation(QVariant idx, QVariant value);
private:
    Ui::DualArmViewWidget *ui;
    std::array<GComponent::Joint*, 7> lJoints;
    std::array<GComponent::Joint*, 7> rJoints;
};

#endif // DUALARMVIEWWIDGET_H
