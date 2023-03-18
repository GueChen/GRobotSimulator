#include "ui/menu/componentmenu.h"

namespace GComponent{

ComponentMenu::ComponentMenu(QWidget* parent):
	QMenu(parent)
{
	m_reset_action            = addAction("Reset");                                           // Reset
    addSeparator();                                                                               // ----------------
    m_remove_component_action = addAction("Remove Component");                                // Remove Component
    m_move_up_action          = addAction("Move Up");                                         // Move Up
    m_move_down_action        = addAction("Move Down");                                       // Move Down
    m_copy_component_actoin   = addAction("Copy Component");                                  // Copy Component
    m_paste_component_action  = addAction("Paste Component");                                 // Paste Component
    addSeparator();                                                                               // ----------------
    m_properties_action       = addAction("Properties");                                      // Properties
    addSeparator();                                                                               // ----------------
    m_add_component_menu      = addMenu("Add Component");                                     // Add Component |>    
    m_add_joint_component         = m_add_component_menu->addAction("Joint Component");       //                  || Joint Component
    m_add_joint_group_component   = m_add_component_menu->addAction("Joint Group Component"); //                  || Joint Group Component
    m_add_kinematic_component     = m_add_component_menu->addAction("Kinematic Component");   //                  || Kinematic Component
    m_add_tracker_component       = m_add_component_menu->addAction("Tracker Component");     // 

    
    
}


}   // !namespace GComponent
