/**
 *  @file  	component_ui_factory.h
 *  @brief 	This Class use to Extraction the Component data to UI.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 25, 2022
 **/
#ifndef _COMPONENT_UI_FACTORY_H
#define _COMPONENT_UI_FACTORY_H

#include "function/widgetbuilder.h"

#include <string>

namespace GComponent {
class ComponentUIFactory {
public:
	ComponentUIFactory() = delete;
	// FIXME: 工厂负责生产不负责销毁，若无接收值发生内存泄漏
	[[nodiscard]]
	static QWidget* Create(const std::string_view& s) {
		QWidgetBuilder builder;
		if (s == "JointComponent") {
			builder.AddLabel("This is Joint", "joint_label")
				.AddLineEdit("0.0", "joint_edit")
				.AddSpacer(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
			return builder.GetWidget();
		}
		if (s == "JointGroupComponent") {
			builder.AddLabel("This is Joint Group", "j_group_label")
				.AddLineEdit("0.0", "j_group_edit")
				.AddSpacer(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
			return builder.GetWidget();
		}
		return nullptr;
	}
};
}

#endif // !_COMPONENT_UI_FACTORY_H
