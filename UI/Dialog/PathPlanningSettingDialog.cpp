#include "PathPlanningSettingDialog.h"
#include "ui_PathPlanningSettingDialog.h"

PathPlanningSettingDialog::PathPlanningSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PathPlanningSettingDialog)
{
    ui->setupUi(this);
}

PathPlanningSettingDialog::~PathPlanningSettingDialog()
{
    delete ui;
}

ThetasPkg PathPlanningSettingDialog::GetThetas() const
{
    Thetas ini, end;

    ini[0] = ui->iniTheta1->text().toDouble();
    ini[1] = ui->iniTheta2->text().toDouble();
    ini[2] = ui->iniTheta3->text().toDouble();
    ini[3] = ui->iniTheta4->text().toDouble();
    ini[4] = ui->iniTheta5->text().toDouble();
    ini[5] = ui->iniTheta6->text().toDouble();
    ini[6] = ui->iniTheta7->text().toDouble();

    end[0] = ui->endTheta1->text().toDouble();
    end[1] = ui->endTheta2->text().toDouble();
    end[2] = ui->endTheta3->text().toDouble();
    end[3] = ui->endTheta4->text().toDouble();
    end[4] = ui->endTheta5->text().toDouble();
    end[5] = ui->endTheta6->text().toDouble();
    end[6] = ui->endTheta7->text().toDouble();

    return {ini, end};
}
