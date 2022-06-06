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

#endif
