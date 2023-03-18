#ifndef _GLOBAL_QSS_H
#define _GLOBAL_QSS_H

#include <string_view>

constexpr std::string_view label_head = 
"QLabel{\n"
	"color:orange;\n"
	"border-radius: 0px;\n"
	"border: 0px none;\n"
	"border-right: 3px double pink;\n"
	"font: 700 10pt \"Microsoft YaHei UI\";\n"
"}";
constexpr std::string_view label_a =
"QLabel{\n"
	"color: rgb(255, 100, 100);\n"
	"border-width: 0px;\n"
"}";
constexpr std::string_view label_alpha = 
"QLabel{\n"
	"color: yellow;\n"
	"border-width: 0px;\n"
"}";
constexpr std::string_view label_d =
"QLabel{\n"
	"color: rgb(255, 100, 255);\n"
	"border-width: 0px;\n"
"}";
constexpr std::string_view label_xita =
"QLabel{\n"
	"color: rgb(100, 220, 255);\n"
	"border-width: 0px;\n"
"}";
constexpr std::string_view line_edit_qss =
"QLineEdit{\n"
	"color:black;\n"
	"background-color: qlineargradient(spread:pad, x1:0.8, y1:0.75, x2:1, y2:1, stop:0 rgba(255, 255, 255, 255), stop:1 rgba(155, 175, 255, 255), );\n"
	"border-radius: 0px;\n"
	"border: 3px double rgb(75, 75, 75);\n"
"}";
constexpr std::string_view widget_qss =
"border: 0px none;";

constexpr std::string_view menu_qss =
"QMenu{\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(0, 0, 0, 200), stop:1 rgba(50, 50, 75, 200));\n"
"	color: white;\n"
"}\n"
"QMenu::item{\n"
"font: 9pt \"微软雅黑\";\n"
"}\n"
"QMenu::indicator:exclusive:checked:selected{\n"
"	border-color: darkblue;\n"
"	background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #6ea1f1, stop:1 #567dbc);\n"
"}\n"
"QMenu::separator{\n"
"\tbackground: lightblue;\n"
"\theight: 1px;\n"
"}";

constexpr std::string_view tabify_dockwidget_qss =
"QTabBar::tab{"
"   min-width: 35px;"
"}"
"QTabBar::tab::selected{"
"   background: rgb(50, 50, 50);"
"   color: qlineargradient(spread:pad, x0: 0, y0: 0, x1: 1, y1: 1, stop: 0 rgb(125, 230, 175), stop: 1 rgb(75, 150, 150));"
"}"
"QTabBar::tab::!selected{"
"   background: rgb(40, 40, 40);"
"}";

constexpr std::string_view normal_button_qss =
"QWidget{"
"background-color: qlineargradient(spread:pad, x1:0, y1:0.75, x2:0, y2:1,"
"stop: 0 rgba(20, 20, 20, 255), stop:1 rgba(45, 45, 65, 255));"
"color:white;"
"}";

constexpr std::string_view red_button_qss =
"QWidget{"
"background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,"
"stop: 0 	 rgba(175, 0, 40, 255),"
"stop: 0.85  rgb(175, 0 ,40),"
"stop: 0.90  rgb(155, 25, 55),"
"stop: 1 	 rgba(155, 25, 55, 255));"
"color:rgb(255, 255, 255);"
"}";

/*_________________________________Component Inspector UI Style Sheet________________________________*/
/// <summary>
/// 用于在 Component 的 UI 中展示 property 的名称时
/// </summary>
constexpr std::string_view component_inspector_text =
"QLabel{"
"color: rgb(255, 255, 255);"
"font: 9pt \"Microsoft YaHei UI\";"
"}";

/// <summary>
/// 用于 LineEditor 的 UI 展示名称使用
/// </summary>
constexpr std::string_view component_inspector_editor =
"QLineEdit{"
"font:  9pt \"Cascadia Code\";"
"color: black;"
"background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
"                                 stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
"                                 stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
"border: 1px solid gray;"
"border-radius: 3px;"
"}";

/// <summary>
/// 用于 ComboBox 的 UI 展示
/// </summary>
constexpr std::string_view combo_editor_qss =
"QComboBox {"
"	font: 9pt \"Microsoft YaHei UI\";"
"	color: black;"
"    border: 1px solid gray;"
"    border-radius: 3px;"
"    padding: 1px 18px 1px 3px;"
"    min-width: 6em;"
"}"
""
"QComboBox:editable {"
"    background: white;"
"}"
""
"QComboBox:!editable, QComboBox::drop-down:editable {"
"     "
"	 "
"     background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
"                                 stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
"                                 stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
"}"
""
"QComboBox:!editable:on, QComboBox::drop-down:editable:on {"
"    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
"                                stop: 0 #D3D3D3, stop: 0.4 #D8D8D8,"
"                                stop: 0.5 #DDDDDD, stop: 1.0 #E1E1E1);"
"}"
""
"QComboBox:on { /* shift the text when the popup opens */"
"    padding-top: 3px;"
"    padding-left: 4px;"
"}"
""
"QComboBox::drop-down {"
"    subcontrol-origin: padding;"
"    subcontrol-position: top right;"
"    width: 15px;"
""
"    border-left-width: 1px;"
"    border-left-color: darkgray;"
"    border-left-style: solid; /* just a single line */"
"    border-top-right-radius: 3px; /* same radius as the QComboBox */"
"    border-bottom-right-radius: 3px;"
"}"
""
"QComboBox::down-arrow {"
"   image:url(:/16x16/downarrow16x16.png)"
"}"
""
"QComboBox::down-arrow:on { /* shift the arrow when popup is open */"
"    top: 1px;"
"    left: 1px;"
"}"
""
"QComboBox QAbstractItemView {"
"	"
"	 background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
"                                 stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
"                                 stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
"    border: 2px solid darkgray;"
"    selection-background-color: lightgray;"
"}";

/// <summary>
/// 用于三维向量中 x 的名称展示
/// </summary>
constexpr std::string_view x_label_qss =
"QLabel{"
"	color:white;"
"	background-color: rgb(220, 0, 0);"
"	border:1px solid;"
"	border-radius: 2px;"
"	border-color: qlineargradient(spread : pad, x1 : 0, y1 : 0, x2 : 1, y2 : 0, stop : 0 rgba(0, 0, 0, 255), stop : 1 rgba(255, 255, 255, 255));"
"	padding: -3 -2px;"
"}";

/// <summary>
/// 用于三维向量中 y 的名称展示
/// </summary>
constexpr std::string_view y_label_qss =
"QLabel{\n"
"	color:white;\n"
"	background-color: rgb(0, 165, 0);\n"
"	border:1px solid;\n"
"	border-radius: 2px;\n"
"	border-color: qlineargradient(spread : pad, x1 : 0, y1 : 0, x2 : 1, y2 : 0, stop : 0 rgba(0, 0, 0, 255), stop : 1 rgba(255, 255, 255, 255));\n"
"	padding: -3 -2px;\n"
"}\n";

/// <summary>
/// 用于三维向量中 z 的名称展示
/// </summary>
constexpr std::string_view z_label_qss =
"QLabel{\n"
"	color:white;\n"
"	background-color: rgb(0, 0, 185);\n"
"	border:1px solid;\n"
"	border-radius: 2px;\n"
"	border-color: qlineargradient(spread : pad, x1 : 0, y1 : 0, x2 : 1, y2 : 0, stop : 0 rgba(0, 0, 0, 255), stop : 1 rgba(255, 255, 255, 255));\n"
"	padding: -3 -2px;\n"
"}\n";

/// <summary>
/// 用于设置 DoubleSpinBox 的风格
/// </summary>
constexpr std::string_view double_spin_box_qss =
"QDoubleSpinBox{"
"	background-color: rgb(255, 255, 255);"
"	border-color: rgb(50, 75, 150);"
"	border-width: 3px;"
"	border-style: groove;"
"}"
"QDoubleSpinBox::hover{"
"	background-color: rgb(200, 200, 225);"
"}";



#endif
