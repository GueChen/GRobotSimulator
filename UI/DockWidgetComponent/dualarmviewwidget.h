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
    class KUKA_IIWA_MODEL;
    class EditorTreeModel;
}

class DualArmViewWidget : public QWidget
{
using TreeModel = GComponent::EditorTreeModel;

Q_OBJECT

private:
    Ui::DualArmViewWidget *ui;
    QTimer          * timer_;
    TreeModel       * treemodel_;

public:
    explicit DualArmViewWidget(QWidget *parent = nullptr);
    ~DualArmViewWidget();

    // TODO: 后期考虑是否修改传递量
    GComponent::KUKA_IIWA_MODEL* getLeftRobot() const;
    GComponent::KUKA_IIWA_MODEL* getRightRobot() const;

    template<class _Deriv_Simplex>
    void addSimplexModel(const std::string& name, unique_ptr<_Deriv_Simplex>&&);
    void clearSimplexModel();

public slots:
    void LeftArmMoveSlot(JointPosFunc posfunc, double T_upper, double period = 0.01);
    void DualArmMoveSlot(DualJointsPosFunc posfunc, double T_upper, double period = 0.01);
};

#endif // DUALARMVIEWWIDGET_H
