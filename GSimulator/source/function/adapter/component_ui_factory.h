/**
 *  @file  	component_ui_factory.h
 *  @brief 	This Class use to Extraction the Component data to UI.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 25, 2022
 **/
#ifndef _COMPONENT_UI_FACTORY_H
#define _COMPONENT_UI_FACTORY_H

#include "pattern/widgetbuilder.h"
#include "base/global/global_qss.h"

#include "component/joint_component.h"
#include "component/joint_group_component.h"
#include "component/kinematic_component.h"
#include "component/tracker_component.h"
#include "component/rigidbody_component.h"
#include "component/material_component.h"

#include "ui/widget/kinematic_widget.h"
#include "ui/widget/tracker_widget.h"
#include "ui/widget/line_edit/drag_accept_edit.h"
#include <GComponent/GNumerical.hpp>

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLayout>
#include <QtWidgets/QDial>
#include <QtWidgets/QToolButton>

#include <string>
#include <unordered_map>
#include <functional>
#ifdef _DEBUG
#include <iostream>
#endif

namespace GComponent {
// TODO: Create a Base Class as Special UI Builder
// Derived from a parent UI Builder

class ComponentUIFactory {

public:
	ComponentUIFactory() = delete;
	// FIXME: 工厂负责生产不负责销毁，若无接收值发生内存泄漏
	[[nodiscard]]
	static QWidget* Create(Component& component) {
		string_view s = component.GetTypeName();
		QWidgetBuilder builder;
		
		if (s == "JointComponent") {
			JointComponent& joint_component = dynamic_cast<JointComponent&>(component);

			/* builder UI Part */
			CreateJointComponentUI(builder);

			/* Connect and Adjust some Datas */									
			ConnectJointComponentUI(builder.GetWidget(), joint_component);
		
			return builder.GetWidget();
		}
		else if (s == "JointGroupComponent") {
			JointGroupComponent& joints_component = dynamic_cast<JointGroupComponent&>(component);
			int num = joints_component.SearchJointsInChildren();

			/* builder UI part */
			CreateJointGroupComponentUI(builder, num);

			/* Connect and Adjust Datas */
			ConnectJointGroupComponentUI(builder.GetWidget(), joints_component);
		
			return builder.GetWidget();
		}
		else if (s == "KinematicComponent") {
			KinematicComponent& kinematic_component = dynamic_cast<KinematicComponent&>(component);
			KinematicComponentWidget* widget = new KinematicComponentWidget;
			ConnectKinematicComponentUI(widget, kinematic_component);
			widget->setGeometry(QRect(0, 0, 300, 600));
			return widget;
		}
		else if (s == "TrackerComponent") {
			TrackerComponent& tracker_component = dynamic_cast<TrackerComponent&>(component);
			TrackerComponentWidget* widget = new TrackerComponentWidget;

			ConnectTrackerComponentUI(widget, tracker_component);
			return widget;
		}
		else if (s == "MaterialComponent") {
			MaterialComponent& material_component = dynamic_cast<MaterialComponent&>(component);
			auto & properties = material_component.GetProperties();
			
			QWidget*	 widget = new QWidget;
			QVBoxLayout* layout = new QVBoxLayout;

			QLabel*		 shader_label = new QLabel("shader");
			shader_label->setMinimumHeight(kEleMiniHeight);
			shader_label->setStyleSheet(component_inspector_text.data());
			layout->addWidget(shader_label);
			
			QLineEdit* shader_editor = new DragAcceptorEditor;
			shader_editor->setMinimumHeight(kEleMiniHeight);
			shader_editor->setText(QString::fromStdString(material_component.GetShader()));						
			layout->addWidget(shader_editor);
			
			for (auto&& pro : properties) {				
				layout->addLayout(build_map[pro.type](pro.name, pro.val));
			}
			
			layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding));
			widget->setLayout(layout);
			widget->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Expanding);
			return widget;
		}
		else{
			// TODO: add the widget
			QWidget* widget = new QWidget;
			return   widget;
		}
		
		return nullptr;
	}

private:
	/*_______________________________The UI Table____________________________________________________*/
	//TODO: as below
	// std::unoredered_map<std::string, unique_ptr<UI_Builder>> ui_table;


	/*_____________________________Joint Component UI Create Methods_________________________________*/
	static void CreateJointComponentUI(QWidgetBuilder& builder);
	static void ConnectJointComponentUI(QWidget* widget, JointComponent& joint_component);

	/*_______________________Joint Group Component UI Create Methods_________________________________*/
	static void CreateJointGroupComponentUI(QWidgetBuilder& builder, int joint_count);
	static void ConnectJointGroupComponentUI(QWidget* widget, JointGroupComponent& component);

	/*___________________________Tracker Component UI Create Methods_________________________________*/
	static void ConnectTrackerComponentUI(TrackerComponentWidget* widget, TrackerComponent& component);

	/*_________________________Kinematic Component UI Create Methods_________________________________*/
	static void ConnectKinematicComponentUI(KinematicComponentWidget* widget, KinematicComponent& component);

	/*_________________________Material Component UI Create Methods__________________________________*/
public:
	static constexpr const int kEleMiniHeight = 20;

private:
	/*_____________________________Joint Component UI Objects________________________________________*/
	static constexpr const string_view JointSliderTagText		= "Joint";
	static constexpr const string_view JointMinLimitTagText		= "min(deg.)";
	static constexpr const string_view JointMaxLimitTagText		= "max(deg.)";
	
	static constexpr const string_view JointSliderObjName		= "joint_slider";
	static constexpr const string_view JointValObjName			= "joint_val";
	static constexpr const string_view JointMaxValObjName		= "max_val";
	static constexpr const string_view JointMinValObjName		= "min_val";
	/*________________________Joint Group Component UI Objects_______________________________________*/
	static constexpr const string_view JointLayoutObjName		= "single_joint_layout";
	static constexpr const string_view JointSliderGroupObjName  = "joint_slider";
	static constexpr const string_view JointGroupValObjName		= "value";
	static constexpr const string_view SettingButtonObjName		= "setting_button";

	static std::unordered_map<std::string, std::function<QLayout*(std::string, ShaderProperty::Var&)>> build_map;
};
} 

#endif // !_COMPONENT_UI_FACTORY_H
