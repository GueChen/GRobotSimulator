#ifndef PATHPLANNINGSETTINGDIALOG_H
#define PATHPLANNINGSETTINGDIALOG_H

#include <QDialog>
#include <array>
#include <tuple>

namespace Ui {
class PathPlanningSettingDialog;
}

using Thetas = std::array<double, 7>;
using ThetasPkg = std::pair<Thetas, Thetas>;


class PathPlanningSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PathPlanningSettingDialog(QWidget *parent = nullptr);
    ~PathPlanningSettingDialog();
    ThetasPkg GetThetas() const;

private:
    Ui::PathPlanningSettingDialog *ui;
};

#endif // PATHPLANNINGSETTINGDIALOG_H
