#ifndef DUALARMVIEWWIDGET_H
#define DUALARMVIEWWIDGET_H

#include <QWidget>
#include <QVariant>
#include <functional>
#include <memory>

using JointPosFunc = std::function<std::vector<double>(double)>;
using DualJointsPosFunc = std::function<std::pair<std::vector<double>,std::vector<double>>(double)>;
using std::unique_ptr;

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

    template<class _Deriv_Simplex>
    void addSimplexModel(const std::string& name, unique_ptr<_Deriv_Simplex>&&);
    void clearSimplexModel();
private:
    Ui::DualArmViewWidget *ui;
    std::array<GComponent::Joint*, 7> lJoints;
    std::array<GComponent::Joint*, 7> rJoints;
    QTimer * myTimer;

public slots:
    void LeftArmMoveSlot(JointPosFunc posfunc, double T_upper, double period = 0.01);
    void DualArmMoveSlot(DualJointsPosFunc posfunc, double T_upper, double period = 0.01);
};

#endif // DUALARMVIEWWIDGET_H
