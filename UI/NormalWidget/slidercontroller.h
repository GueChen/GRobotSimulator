#ifndef SLIDERCONTROLLER_H
#define SLIDERCONTROLLER_H

#include <QWidget>

namespace Ui {
class SliderController;
}

class SliderController : public QWidget
{
    Q_OBJECT

public:
    explicit SliderController(QWidget *parent = nullptr);
    ~SliderController();

private:
    Ui::SliderController *ui;
};

#endif // SLIDERCONTROLLER_H
