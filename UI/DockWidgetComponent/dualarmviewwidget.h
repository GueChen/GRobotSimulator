#ifndef DUALARMVIEWWIDGET_H
#define DUALARMVIEWWIDGET_H

#include <QWidget>
#include <QVariant>

namespace Ui {
class DualArmViewWidget;
}

namespace GComponent{
    class Joint;
    class KUKA_IIWA_MODEL;
}

class DualArmViewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DualArmViewWidget(QWidget *parent = nullptr);
    ~DualArmViewWidget();
    Q_INVOKABLE void setLeftArmRotation(QVariant idx, QVariant value);
    // TODO: 后期考虑是否修改传递量
    GComponent::KUKA_IIWA_MODEL* getLeftRobot() const;
    GComponent::KUKA_IIWA_MODEL* getRightRobot() const;
private:
    Ui::DualArmViewWidget *ui;
    std::array<GComponent::Joint*, 7> lJoints;
    std::array<GComponent::Joint*, 7> rJoints;
};

#endif // DUALARMVIEWWIDGET_H
