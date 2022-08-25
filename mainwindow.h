#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QComboBox>

#include <unordered_map>
#include <functional>
#include <memory>

using std::unique_ptr;
using std::vector;
using std::make_unique;
using UPD = unique_ptr<QDockWidget>;
using Map_StrUPD = std::unordered_map<const std::string, UPD>;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QChartView;
class QChart;
QT_END_NAMESPACE

namespace GComponent{
// UI Related Class
class GLModelTreeView;
class ComponentMenu;
// Function Related Class
class UIState;
class Component;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT        
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    GComponent::UIState*         getUIState()       const;
    GComponent::GLModelTreeView* getModelTreeView() const; 
    QComboBox*                   getModelDispaly()  const;
    GComponent::ComponentMenu*   getComponentMenu() const;

private:
    void ConnectionInit();

public slots:    
    void ResponseComponentCreateRequest(GComponent::Component* component_ptr, const QString& type_name);

private slots:
    void ReceiveDeltaTime(float delta_time);
    void SetTabifyDockerWidgetQSS(QDockWidget* widget);
    void CheckSelected();

/*_________________________Button Click______________________________________________________*/
    void on_check_button_clicked();
    void on_trans_button_clicked();
    void on_rot_button_clicked();
    void on_scale_button_clicked();

/*_________________________Menu Popup_________________________________________________________*/
    void on_componentstoolbox_customContextMenuRequested(const QPoint& pos);

signals:
    void RequestDeleteComponent(const QString& component_name);
    void RequestAddComponent(const QString& component_name);
    void RequestShowDialog(const QString& dialog_name);

private:

    Ui::MainWindow*             ui_;
    QTimer*                     updated_timer_ptr_;

/*_______________________Components MENU______________________________________________________*/
    GComponent::ComponentMenu*  component_menu_;    
};
#endif // MAINWINDOW_H
