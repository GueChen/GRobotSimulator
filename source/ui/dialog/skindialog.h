/**
* 
* 
* 
* 
**/
#ifndef __SKIN_DIALOG_H
#define __SKIN_DIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/qcombobox>
#include <QtWidgets/QGraphicsWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGraphicsGridLayout>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGroupBox>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE
namespace Ui {
	class SkinDialog;
}
QT_END_NAMESPACE

namespace GComponent {

	class SkinDialog : public QDialog
	{
		Q_OBJECT

	public:
		explicit SkinDialog(QWidget* parent = nullptr);
		~SkinDialog();

        QComboBox* ComboText();

    public slots:
        void DisplayValue(QList<QString> str);
        void GetValue(QList<float> v);
        void GetPortlist(QList<QString> namelist);

    private slots:
        void on_startButton_clicked();
        void on_zeroButton_clicked();
        void on_comboBox_currentTextChanged(const QString& arg1);        
        void on_readportButton_clicked();
        void on_setButton_clicked();

    private:
        Ui::SkinDialog*         skin_ui;
        QList<float>            value;
        QChart*                 m_chart;
        QBarSet*                set;
        QBarSeries*             series;
        QBarCategoryAxis*       axisX;
        QValueAxis*             axisY;
        QChartView*             chartView;
        QVBoxLayout*            pVLayout;
        int                     _FPS = 0;

        void DrawChart(QList<float> v);
        void InitChart();
        std::vector<float> GetOneColFromTable(QTableWidget* tbl, int col_idx);
        std::vector<float> GetPositionFromTable(QTableWidget* tbl);
        std::vector<float> GetRotationFromTable(QTableWidget* tbl);

    signals:
        void GetPortName(QString currentName);
        void RunCtr();
        void CloseCtr();
        void SetZero();
        void SetTrue();
        void ReadPort();
        void SetDirectionVector(int unitname, int count, std::vector<float> tar_dir);
	};
	
}

#endif