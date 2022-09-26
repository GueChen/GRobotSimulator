#ifndef _COMPONENTMENU_H
#define _COMPONENTMENU_H

#include <QtWidgets/QMenu>

namespace GComponent {

class ComponentMenu : public QMenu
{
public:
    explicit ComponentMenu(QWidget* parent = nullptr);

public:
    QAction*    m_reset_action;
    QAction*    m_remove_component_action;
    QAction*    m_move_up_action;
    QAction*    m_move_down_action;
    QAction*    m_copy_component_actoin;
    QAction*    m_paste_component_action;
    QAction*    m_properties_action;
    QMenu*      m_add_component_menu;
    QAction*    m_add_joint_component;
    QAction*    m_add_joint_group_component;
    QAction*    m_add_kinematic_component;
    QAction*    m_add_tracker_component;
};

} // !namespace GComponent
#endif // !_COMPONENTMENU_H
