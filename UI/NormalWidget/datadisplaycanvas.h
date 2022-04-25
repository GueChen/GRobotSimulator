#ifndef DATADISPLAYCANVAS_H
#define DATADISPLAYCANVAS_H

#include <QtWidgets/QWidget>

namespace Ui {
class DataDisplayCanvas;
}

class DataDisplayCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit DataDisplayCanvas(QWidget *parent = nullptr);
    ~DataDisplayCanvas();

private:
    Ui::DataDisplayCanvas *ui;
};

#endif // DATADISPLAYCANVAS_H
