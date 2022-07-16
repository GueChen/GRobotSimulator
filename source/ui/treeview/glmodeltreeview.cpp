#include "ui/treeview/glmodeltreeview.h"

#include "base/global/global_qss.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QkeyEvent>

#include <iostream>
      
namespace GComponent{

GLModelTreeView::GLModelTreeView(QWidget *parent)
	: QTreeView(parent)
{
    setFocusPolicy(Qt::ClickFocus);
    InitMenuActions();
   
    /* Set Style Sheet */
    m_basic_menu.setStyleSheet(menu_qss.data());
    m_add_menu.setStyleSheet(menu_qss.data());
   
}   

GLModelTreeView::~GLModelTreeView()
{

}

void GLModelTreeView::setModel(QAbstractItemModel* model)
{
    QTreeView::setModel(model);
    /* this << selection */
    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, 
            this,             &GLModelTreeView::SelectionChangeSlot);
}

void GLModelTreeView::mouseReleaseEvent(QMouseEvent* event)
{
    QTreeView::mouseReleaseEvent(event);
    if (event->button() == Qt::RightButton)
    {        
        m_basic_menu.popup(event->globalPos());
    }
}

void GLModelTreeView::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete) 
    {
        emit DeleteRequest(selectionModel()->currentIndex().data().toString().toStdString());
    }
}

void GLModelTreeView::InitMenuActions()
{
    m_add_menu.setTitle("3D Objects");

    copy_action_         = new QAction("copy");
    cut_action_          = new QAction("cut");
    paste_action_        = new QAction("paste");
    delete_action_       = new QAction("delete");
    rename_action_       = new QAction("rename");
    robot_create_action_ = new QAction("Robot");

    m_basic_menu.addAction(copy_action_);
    m_basic_menu.addAction(cut_action_);
    m_basic_menu.addAction(paste_action_);
    m_basic_menu.addSeparator();
    m_basic_menu.addAction(delete_action_);
    m_basic_menu.addAction(rename_action_);
    m_basic_menu.addSeparator();
    m_basic_menu.addAction(m_add_menu.menuAction());
    m_basic_menu.addAction(robot_create_action_);

    cube_create_action_     = new QAction("Cube");
    sphere_create_action_   = new QAction("Sphere");
    cylinder_create_action_ = new QAction("Cylinder");
    capsule_create_action_  = new QAction("Capsule");
    plane_create_action_    = new QAction("Plane");
    
    m_add_menu.addAction(cube_create_action_);
    m_add_menu.addAction(sphere_create_action_);
    m_add_menu.addAction(cylinder_create_action_);
    m_add_menu.addAction(capsule_create_action_);
    m_add_menu.addAction(plane_create_action_);

    connect(cube_create_action_,     &QAction::triggered, [this]() { emit CreateRequest("cube"); });
    connect(sphere_create_action_,   &QAction::triggered, [this]() { emit CreateRequest("sphere"); });
    connect(cylinder_create_action_, &QAction::triggered, [this]() { emit CreateRequest("cylinder"); });
    connect(capsule_create_action_,  &QAction::triggered, [this]() { emit CreateRequest("capsule"); });
    connect(plane_create_action_,    &QAction::triggered, [this]() { emit CreateRequest("plane"); });
    connect(robot_create_action_,    &QAction::triggered, [this]() { emit RobotCreateRequest();});

    copy_action_->setEnabled(false);
    cut_action_->setEnabled(false);
    paste_action_->setEnabled(false);

    delete_action_->setEnabled(false);
    rename_action_->setEnabled(false);
}

/*_________________________________________SLOT FUNCTIONS_____________________________________________*/
void GLModelTreeView::ResponseSelectRequest(const QModelIndex& select)
{
    setCurrentIndex(select);   
}

void GLModelTreeView::SelectionChangeSlot(const QModelIndex& current, const QModelIndex& previous)
{
    emit SelectRequest(current.data().toString().toStdString());
}

void GLModelTreeView::ResponseDataDeleted()
{
    setCurrentIndex(QModelIndex());
}

}
