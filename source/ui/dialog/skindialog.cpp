#include "skindialog.h"
#include "ui_skindialog.h"

using std::vector;

namespace GComponent {

SkinDialog::SkinDialog(QWidget* parent) :
    QDialog(parent),
    skin_ui(new Ui::SkinDialog)

{
    skin_ui->setupUi(this);
    InitChart();
}

SkinDialog::~SkinDialog()
{
    delete skin_ui;
}

/*______________________________SLOTS METHODS__________________________________*/
/*______________________________PRIVATE METHODS_________________________________*/
void SkinDialog::on_startButton_clicked()
{
    if (skin_ui->startButton->text() == tr("start"))
    {
        skin_ui->startButton->setText(tr("stop"));
        emit RunCtr();

    }
    else
    {
        skin_ui->startButton->setText(tr("start"));
        emit CloseCtr();
    }
}

void SkinDialog::on_zeroButton_clicked()
{
    if (skin_ui->zeroButton->text() == tr("standardize"))
    {
        skin_ui->zeroButton->setText(tr("true"));
        emit SetZero();
    }
    else
    {
        skin_ui->zeroButton->setText(tr("standardize"));
        emit SetTrue();
    }
}

void SkinDialog::on_comboBox_currentTextChanged(const QString& arg1)
{
    emit GetPortName(arg1);
}

void SkinDialog::on_readportButton_clicked()
{
    emit ReadPort();
}

void SkinDialog::on_setButton_clicked()
{
    int unitname = skin_ui->unitBox->currentIndex();
    int count = skin_ui->countBox->currentText().toInt();
    vector<float> tar_dir = GetRotationFromTable(skin_ui->basis_vec_table);
    emit SetDirectionVector(unitname, count, tar_dir);
}
/*______________________________PUBLIC METHODS_________________________________*/
void SkinDialog::DisplayValue(QList<QString> str)
{
    skin_ui->textEdit->clear();
    skin_ui->textEdit_2->clear();
    skin_ui->textEdit_3->clear();
    skin_ui->textEdit_4->clear();
    skin_ui->textEdit_5->clear();
    skin_ui->textEdit_6->clear();
    skin_ui->textEdit_7->clear();
    skin_ui->textEdit_8->clear();
    skin_ui->textEdit->append(str[0]);
    skin_ui->textEdit_2->append(str[1]);
    skin_ui->textEdit_3->append(str[2]);
    skin_ui->textEdit_4->append(str[3]);
    skin_ui->textEdit_5->append(str[4]);
    skin_ui->textEdit_6->append(str[5]);
    skin_ui->textEdit_7->append(str[6]);
    skin_ui->textEdit_8->append(str[7]);
}

void SkinDialog::GetValue(QList<float> v)
{
    value = v;
    ++_FPS;
    if (_FPS >= 10)
    {
        DrawChart(value);
        _FPS = 0;
    }    
}

void SkinDialog::GetPortlist(QList<QString> namelist)
{
    skin_ui->comboBox->clear();
    for (int i = 0; i < namelist.size(); i++)
    {
        skin_ui->comboBox->addItem(namelist[i]);
    }
}
/*__________________________________PUBLIC______________________________________*/
QComboBox* SkinDialog::ComboText()
{
    return (skin_ui->comboBox);
}

/*__________________________________PRIVATE_____________________________________*/
void SkinDialog::DrawChart(QList<float> v)
{
    QList<qreal> bar = QList<qreal>(8);
    for (int i = 0; i < 8; i++)
    {
        bar[i] = v[i];
        //set->replace(i, v[i]);//emit 8 times
    }
    set->remove(0,8);//emit 1 times
    set->append(bar);//emit 1 times    
    
}

void SkinDialog::InitChart()
{
    m_chart = new QChart();
    set = new QBarSet("");
    series = new QBarSeries();
    axisX = new QBarCategoryAxis;
    axisY = new QValueAxis;
    chartView = skin_ui->graphicsView;
    //chartView = new QChartView(/*m_chart*/);    
    //pVLayout = new QVBoxLayout(this);

    *set << 0 << 0 << 0 << 0 << 0 << 0 << 0 << 0;
    series->append(set);

    axisX->append("unit_1");
    axisX->append("unit_2");
    axisX->append("unit_3");
    axisX->append("unit_4");
    axisX->append("unit_5");
    axisX->append("unit_6");
    axisX->append("unit_7");
    axisX->append("unit_8");
    axisX->setLabelsColor(QColor(7, 28, 96));

    axisY->setRange(0, 4);
    axisY->setTitleText("V");
    axisY->setLabelFormat("%d");

    m_chart->addSeries(series);
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    set->setLabelColor(QColor(0, 0, 255));

    series->setVisible(true);
    series->setLabelsVisible(true);

    m_chart->setDropShadowEnabled(true);
    m_chart->setTheme(QChart::ChartThemeLight);
    m_chart->setTitleBrush(QBrush(QColor(0, 0, 255)));
    //m_chart->setTitleFont(QFont(" Î¢ÈíÑÅºÚ "));
    m_chart->setTitle(" Skin sensor signal ");

    chartView->setChart(m_chart);
    //skin_ui->graphicsView = chartView;
    //pVLayout->setContentsMargins(5, 40, 180, 40);
    //pVLayout->addWidget(chartView);

}

vector<float> SkinDialog::GetOneColFromTable(QTableWidget* tbl, int col_idx)
{
    if (!tbl) return vector<float>{};
    vector<float> vals(tbl->rowCount());
    for (int i = 0; i < vals.size(); ++i) {
        vals[i] = tbl->item(i,col_idx)->text().toFloat();
    }
    return vals;
}

vector<float> SkinDialog::GetPositionFromTable(QTableWidget* tbl)
{
    if (!tbl) return vector<float>{};
    vector<float> vals(3);
    for (int i = 0; i < 3; ++i) {
        vals[i] = tbl->item(i, 0)->text().toFloat();
    }
    return vals;
}

vector<float> SkinDialog::GetRotationFromTable(QTableWidget* tbl)
{
    if (!tbl) return vector<float>{};
    vector<float> vals(3);
    for (int i = 0; i < 3; ++i) {
        vals[i] = tbl->item(i + 3, 0)->text().toFloat();
    }
    return vals;
}




}