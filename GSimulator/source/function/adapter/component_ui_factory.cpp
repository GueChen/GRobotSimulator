#include "function/adapter/component_ui_factory.h"
#include "function/qconversion.hpp"

#include "base/global/global_qss.h"

#include "manager/resourcemanager.h"

#include "component/Components.h"

#include "ui/widget/kinematic_widget.h"
#include "ui/widget/tracker_widget.h"

#include "ColorEdit/color_edit.h"
#include "Selector/selector.h"
#include "Vector3Edit/vector3_edit.h"

#include <Eigen/Geometry>

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtWidgets/QDial>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QComboBox>

#include <QtGui/QDropEvent>
#include <QtGui/QDragEnterEvent>

#include <any>
#include <iostream>

namespace GComponent {
using std::string;

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

/*_____________________________Joint Component UI Objects________________________________________*/
static constexpr const string_view JointSliderTagText = "Joint";
static constexpr const string_view JointMinLimitTagText = "min(deg.)";
static constexpr const string_view JointMaxLimitTagText = "max(deg.)";

static constexpr const string_view JointSliderObjName = "joint_slider";
static constexpr const string_view JointValObjName = "joint_val";
static constexpr const string_view JointMaxValObjName = "max_val";
static constexpr const string_view JointMinValObjName = "min_val";
/*________________________Joint Group Component UI Objects_______________________________________*/
static constexpr const string_view JointLayoutObjName = "single_joint_layout";
static constexpr const string_view JointSliderGroupObjName = "joint_slider";
static constexpr const string_view JointGroupValObjName = "value";
static constexpr const string_view SettingButtonObjName = "setting_button";

/*________________________Create Standard Methods_________________________________________________*/
static QLabel* CreateStandardTextLabel(const std::string& name) {
	QLabel* name_text = new QLabel(QString::fromStdString(name));
	name_text->setStyleSheet(component_inspector_text.data());
	name_text->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
	name_text->setMinimumHeight(ComponentUIFactory::kEleMiniHeight);
	return name_text;
}

static QHBoxLayout* CreateStanardCheckBox(bool & flag) {
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));

	QCheckBox* checkbox = new QCheckBox;
	checkbox->setChecked(flag);
	QObject::connect(checkbox, &QCheckBox::stateChanged, [&val = flag](bool value) { val = value; });
	layout->addWidget(checkbox);
	return layout;
}

static QHBoxLayout* CreateStanardCheckBox(std::function<bool(void)> getter, std::function<void(bool)> setter) {
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));

	QCheckBox* checkbox = new QCheckBox;
	checkbox->setChecked(getter());
	QObject::connect(checkbox, &QCheckBox::stateChanged, [setter = setter](bool value) { setter(value); });
	layout->addWidget(checkbox);
	return layout;
}

static QLineEdit* CreateStandardLineEdit(const std::string& name, bool read_only = true) {
	QLineEdit* line_edit = new QLineEdit(QString::fromStdString(name));
	
	line_edit->setStyleSheet(component_inspector_editor.data());
	line_edit->setReadOnly(read_only);
	line_edit->setAlignment(Qt::AlignHCenter);

	return line_edit;
}

static QVBoxLayout* CreateVector3Editor(const std::string& name,
										std::function<Eigen::Vector3f(void)>		getter,
										std::function<void(const Eigen::Vector3f&)> setter,
										float lower, float upper, float step) {
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(CreateStandardTextLabel(name));	
	
	Vector3Edit* editor = new Vector3Edit(nullptr, QConversion::fromVec3f(getter()));
	editor->SetRange(lower, upper);	
	editor->SetStep (step);
	QObject::connect(editor, &Vector3Edit::VectorChanged, [setter = setter](const QVector3D& value) {		
		setter(QConversion::toVec3f(value));
	});
	layout->addWidget(editor);

	return layout;	
}

/*___________________________________Normal Variables Builder Map___________________________________*/
// TODO: complete this and remove redundancy map
static unordered_map<std::string, std::function<QLayout* (const std::string&, std::any)>> normal_builder_map = {
	{"int",
	[](const std::string& name, std::any val)->QLayout* {
		QHBoxLayout* layout = new QHBoxLayout;
		layout->addWidget(CreateStandardTextLabel(name));
		QSpinBox* spinbox = new QSpinBox;
		spinbox->setValue(*std::any_cast<int*>(val));
		QObject::connect(spinbox, &QSpinBox::valueChanged, [val = val](int value) { (* std::any_cast<int*>(val)) = value; });
		layout->addWidget(spinbox);
		return layout;
	}},
	{"float",
	[](const std::string& name, std::any val)->QLayout* {
		QHBoxLayout* layout = new QHBoxLayout;
		layout->addWidget(CreateStandardTextLabel(name));
		QDoubleSpinBox* spinbox = new QDoubleSpinBox;
		spinbox->setValue(*std::any_cast<float*>(val));
		spinbox->setStyleSheet(double_spin_box_qss.data());
		QObject::connect(spinbox, &QDoubleSpinBox::valueChanged,[val = val](double value) { (*std::any_cast<float*>(val)) = value; });
		layout->addWidget(spinbox);
		return layout;
	}}
};

/*___________________________________Material Shader Uniform Variables______________________________*/
// ui Create table acoording the class with Shader Properties
std::unordered_map<std::string, std::function<QLayout*(std::string, ShaderProperty::Var&)>> ComponentUIFactory::build_map = {
	{"unsigned int",
	[](std::string name, ShaderProperty::Var& val)->QLayout* {
		QHBoxLayout* layout = new QHBoxLayout;

		layout->addWidget(CreateStandardTextLabel(name));

		QSpinBox* spinbox = new QSpinBox;
		spinbox->setValue(std::get<unsigned int>(val));
		spinbox->setMinimum(0);
		QObject::connect(spinbox, &QSpinBox::valueChanged, [&val = val](int value) { val = value; });
		layout->addWidget(spinbox);
		return layout;
	}},
	{"int", 
	 [](std::string name, ShaderProperty::Var& val)->QLayout*{
		QHBoxLayout* layout = new QHBoxLayout;
				
		layout->addWidget(CreateStandardTextLabel(name));

		QSpinBox* spinbox = new QSpinBox;
		spinbox->setValue(std::get<int>(val));
		QObject::connect(spinbox, &QSpinBox::valueChanged, [&val = val](int value) { val = value;});
		layout->addWidget(spinbox);
		return layout;
	}},
	{"bool",
	[](std::string name, ShaderProperty::Var& val)->QLayout* {
		QVBoxLayout* layout = new QVBoxLayout;
		layout->addWidget(CreateStandardTextLabel(name));
		layout->addLayout(CreateStanardCheckBox(std::get<bool>(val)));
		return layout;
	}},
	{"float",
	[](std::string name, ShaderProperty::Var& val)->QLayout* {
		QHBoxLayout* layout = new QHBoxLayout;
		
		QLayout* float_layout  = normal_builder_map["float"](name, std::any(&std::get<float>(val)));		
				
		for (int i = 0; i < float_layout->count(); ++i) {
			QLayoutItem* item = float_layout->itemAt(i);
			if (auto spin_box = dynamic_cast<QDoubleSpinBox*>(item->widget())) {
				spin_box->setMinimum(0.0);
				spin_box->setMaximum(1.0);
				spin_box->setSingleStep(0.01);
			}
		}				
		layout->addLayout(float_layout);		
		return layout;		
	}},
	{"vec3",
	[](std::string name, ShaderProperty::Var& val)->QLayout* {
		glm::vec3& value = std::get<glm::vec3>(val);
		
		QVBoxLayout* layout = new QVBoxLayout;
		
		layout->addWidget(CreateStandardTextLabel(name));

		QHBoxLayout* sub_layout = new QHBoxLayout;
		QLabel* x_label = new QLabel("x");
		x_label->setAlignment(Qt::AlignCenter);
		x_label->setStyleSheet(x_label_qss.data());
		x_label->setMinimumSize(QSize(kEleMiniHeight, kEleMiniHeight));
		sub_layout->addWidget(x_label);
		
		QDoubleSpinBox* x_spinbox = new QDoubleSpinBox;
		x_spinbox->setValue(value.x);
		x_spinbox->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
		x_spinbox->setStyleSheet(double_spin_box_qss.data());
		QObject::connect(x_spinbox, &QDoubleSpinBox::valueChanged, [&x = value.x](double value) { x = value;});
		sub_layout->addWidget(x_spinbox);

		QLabel* y_label = new QLabel("y");
		y_label->setAlignment(Qt::AlignCenter);
		y_label->setStyleSheet(y_label_qss.data());
		y_label->setMinimumSize(QSize(kEleMiniHeight, kEleMiniHeight));
		sub_layout->addWidget(y_label);

		QDoubleSpinBox* y_spinbox = new QDoubleSpinBox;
		y_spinbox->setValue(value.y);
		y_spinbox->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
		y_spinbox->setStyleSheet(double_spin_box_qss.data());
		QObject::connect(y_spinbox, &QDoubleSpinBox::valueChanged, [&y = value.y](double value) { y = value; });
		sub_layout->addWidget(y_spinbox);

		QLabel* z_label = new QLabel("z");
		z_label->setAlignment(Qt::AlignCenter);
		z_label->setMinimumSize(QSize(kEleMiniHeight, kEleMiniHeight));
		z_label->setStyleSheet(z_label_qss.data());
		sub_layout->addWidget(z_label);

		QDoubleSpinBox* z_spinbox = new QDoubleSpinBox;
		z_spinbox->setValue(value.z);
		z_spinbox->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
		z_spinbox->setStyleSheet(double_spin_box_qss.data());
		QObject::connect(z_spinbox, &QDoubleSpinBox::valueChanged, [&z = value.z](double value) { z = value; });
		sub_layout->addWidget(z_spinbox);

		layout->addLayout(sub_layout);
		return layout;
	}},
	{"color",
	[](std::string name, ShaderProperty::Var& val)->QLayout* {
		glm::vec3& value = std::get<glm::vec3>(val);
		QColor color; 
		color.setRedF  (value.x);
		color.setGreenF(value.y);
		color.setBlueF (value.z);
		
		QVBoxLayout* layout = new QVBoxLayout;
		layout->addWidget(CreateStandardTextLabel(name));
		
		ColorEdit* color_edit = new ColorEdit(nullptr, color);
		QObject::connect(color_edit, &ColorEdit::ColorChanged, [&val = value](const QColor& color) {
			val.x = color.redF();
			val.y = color.greenF();
			val.z = color.blueF();
		});

		layout->addWidget(color_edit);
		return layout;
	}},
	{"mat4", [](std::string name, ShaderProperty::Var& val)->QLayout* {return nullptr; }},
	{"sampler2D",
	 [](std::string name, ShaderProperty::Var& val)->QLayout* {
			QVBoxLayout* layout = new QVBoxLayout;
			layout->addWidget(CreateStandardTextLabel(name));

			QHBoxLayout* sub_layout = new QHBoxLayout;
			sub_layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));						
			sub_layout->addWidget(new ColorEdit(nullptr, QColor(Qt::white)));
			layout->addLayout(sub_layout);
			return layout;
	}},
	{"samplerCUBE",
	 [](std::string name, ShaderProperty::Var& val)->QLayout* {
			QVBoxLayout* layout = new QVBoxLayout;
			layout->addWidget(CreateStandardTextLabel(name));

			QHBoxLayout* sub_layout = new QHBoxLayout;
			sub_layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));
			sub_layout->addWidget(new ColorEdit(nullptr, QColor(Qt::white)));
			layout->addLayout(sub_layout);
			return layout;
	} },
	{"sampler2DArray",
	 [](std::string name, ShaderProperty::Var& val)->QLayout* {
			QVBoxLayout* layout = new QVBoxLayout;
			layout->addWidget(CreateStandardTextLabel(name));

			QHBoxLayout* sub_layout = new QHBoxLayout;
			sub_layout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding));
			sub_layout->addWidget(new ColorEdit(nullptr, QColor(Qt::white)));
			layout->addLayout(sub_layout);
			return layout;
	} }
};

/*___________________________________Collider Shapes Bulid Map______________________________________*/
static std::map<ShapeEnum, std::function<QLayout* (AbstractShape*)>> shapes_builder_map = {
#define SetFloatLimitation(ptr, minimal, maxmall, single) \
	do{													  \
		(ptr)->setMinimum(minimal);						  \
		(ptr)->setMaximum(maxmall);						  \
		(ptr)->setSingleStep(single);					  \
	} while(0)

#define AddShapeDisplayer(layout_ptr, ShapeType)						\
	do {																\
		(layout_ptr)->addWidget(CreateStandardTextLabel("shape type"));	\
		(layout_ptr)->addWidget(CreateStandardLineEdit(#ShapeType));	\
	} while(0)

#define FindTypeInLayout(ret, layout, Type)								\
	do{																	\
		for (int __i = 0; __i < layout->count(); ++__i) {				\
			QLayoutItem* __item = layout->itemAt(__i);					\
			if (auto __ptr = dynamic_cast<Type*>(__item->widget())) {	\
				ret = __ptr;											\
				break;													\
			}															\
		}																\
	} while(0)

	{ShapeEnum::Sphere,
	[](AbstractShape* shape)->QLayout* {
		SphereShape*	sphere = dynamic_cast<SphereShape*>(shape);
		QVBoxLayout*	layout = new QVBoxLayout;
		AddShapeDisplayer(layout, Sphere);

		QLayout*		radius_layout = normal_builder_map["float"]("radius", std::any(&sphere->m_radius));
		QDoubleSpinBox* spinbox = nullptr;
		FindTypeInLayout(spinbox, radius_layout, QDoubleSpinBox);
		SetFloatLimitation(spinbox, 0.0, 999999.9, 0.01);
		layout->addLayout(radius_layout);
		
		return layout;
	}},
	{ShapeEnum::Capsule,
	[](AbstractShape* shape)->QLayout*{
		CapsuleShape* capsule = dynamic_cast<CapsuleShape*>(shape);

		QVBoxLayout* layout = new QVBoxLayout;
		AddShapeDisplayer(layout, Capsule);

		QLayout* radius_layout = normal_builder_map["float"]("radius", std::any(&capsule->m_radius));
		{
			QDoubleSpinBox* spinbox = nullptr;
			FindTypeInLayout(spinbox, radius_layout, QDoubleSpinBox);
			SetFloatLimitation(spinbox, 0.0, 999999.9, 0.01);
		}
		layout->addLayout(radius_layout);

		QLayout* half_height_layout = normal_builder_map["float"]("half height", std::any(&capsule->m_radius));
		{
			QDoubleSpinBox* spinbox = nullptr;
			FindTypeInLayout(spinbox, half_height_layout, QDoubleSpinBox);
			SetFloatLimitation(spinbox, 0.0, 999999.9, 0.01);
		}
		layout->addLayout(half_height_layout);

		return layout;
	}},
	{ShapeEnum::Box,
	[](AbstractShape* shape)->QLayout* {
		BoxShape* box = dynamic_cast<BoxShape*>(shape);
		
		QVBoxLayout* layout = new QVBoxLayout;		
		AddShapeDisplayer(layout, Box);

		QLayout* half_x_layout = normal_builder_map["float"]("half x", std::any(&box->m_half_x));
		{
			QDoubleSpinBox* spinbox = nullptr;
			FindTypeInLayout(spinbox, half_x_layout, QDoubleSpinBox);
			SetFloatLimitation(spinbox, 0.0, 999999.9, 0.01);
		}
		layout->addLayout(half_x_layout);

		QLayout* half_y_layout = normal_builder_map["float"]("half y", std::any(&box->m_half_y));
		{
			QDoubleSpinBox* spinbox = nullptr;
			FindTypeInLayout(spinbox, half_y_layout, QDoubleSpinBox);
			SetFloatLimitation(spinbox, 0.0, 999999.9, 0.01);
		}
		layout->addLayout(half_y_layout);
		
		QLayout* half_z_layout = normal_builder_map["float"]("half z", std::any(&box->m_half_z));
		{
			QDoubleSpinBox* spinbox = nullptr;
			FindTypeInLayout(spinbox, half_z_layout, QDoubleSpinBox);
			SetFloatLimitation(spinbox, 0.0, 999999.9, 0.01);
		}
		layout->addLayout(half_z_layout);

		return layout;
	}},
	{ShapeEnum::ConvexHull,
	[](AbstractShape* shape)->QLayout* {
		ConvexShape* convex = dynamic_cast<ConvexShape*>(shape);

		QVBoxLayout* layout = new QVBoxLayout;
		AddShapeDisplayer(layout, ConvexHull);

		layout->addWidget(CreateStandardTextLabel("vert size"));
		layout->addWidget(CreateStandardLineEdit(std::to_string(convex->m_positions.size())));

		layout->addWidget(CreateStandardTextLabel("face size"));
		layout->addWidget(CreateStandardLineEdit(std::to_string(convex->m_faces.size())));

		return layout;
	}}

#undef AddShapeDisplayer
#undef FindTypeInLayout
#undef SetFloatLimitation
};

/*______________________Joint Component UI BUILDER METHODS__________________________________________*/
void CreateJointComponentUI(QWidgetBuilder& builder) {
	builder.GetWidgetLayout()->addWidget(new QDial);

	QSlider* slider = new QSlider(Qt::Horizontal);
	//slider->setParent(&boxlayout);
	slider->setObjectName(JointSliderObjName.data());
	slider->setMinimum(-1);
	slider->setMaximum(1);
	slider->setValue(0);
	builder.GetWidgetLayout()->addWidget(slider);

	QHBoxLayout* sliderlayout = new QHBoxLayout;
	// add slider
	/* |label | | s--l--i--d--e--r| | | label | */
	QLabel* val_tag = new QLabel(JointSliderTagText.data());
	val_tag->setObjectName("val_tag");
	val_tag->setMinimumWidth(75);
	sliderlayout->addWidget(val_tag);

	QLineEdit* val = new QLineEdit("0.0");		
	val->setObjectName(JointValObjName.data());
	val->setAlignment(Qt::AlignCenter);
	sliderlayout->addWidget(val);

	builder.AddLayout(sliderlayout);

	// add label val
	QHBoxLayout* min_limit_layout = new QHBoxLayout;

	QLabel* min_limit_label = new QLabel(JointMinLimitTagText.data());
	min_limit_label->setObjectName("min_label");
	min_limit_label->setMinimumWidth(75);
	min_limit_layout->addWidget(min_limit_label);

	QLineEdit* min_val = new QLineEdit("-180.0");
	min_val->setObjectName(JointMinValObjName.data());
	min_val->setAlignment(Qt::AlignCenter);
	min_limit_layout->addWidget(min_val);

	builder.AddLayout(min_limit_layout);

	// add label val
	QHBoxLayout* max_limit_layout = new QHBoxLayout;

	QLabel* max_limit_label = new QLabel(JointMaxLimitTagText.data());
	max_limit_label->setObjectName("max_label");
	max_limit_label->setMinimumWidth(75);
	max_limit_layout->addWidget(max_limit_label);

	QLineEdit* max_val = new QLineEdit("180.0");
	max_val->setObjectName(JointMaxValObjName.data());
	max_val->setAlignment(Qt::AlignCenter);
	max_limit_layout->addWidget(max_val);

	builder.AddLayout(max_limit_layout);

	builder.AddSpacer(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
	
}

void ConnectJointComponentUI(QWidget* widget, JointComponent& joint_component)
{
	QSlider* slider	   = widget->findChild<QSlider*>(JointSliderObjName.data());
	QLineEdit* val	   = widget->findChild<QLineEdit*>(JointValObjName.data());
	QLineEdit* max_val = widget->findChild<QLineEdit*>(JointMaxValObjName.data());
	QLineEdit* min_val = widget->findChild<QLineEdit*>(JointMinValObjName.data());

	// Component >> UI
	joint_component.RegisterUpdateFunction(ComponentUIFactory::kTag, [val, &joint_component](float delta) {
		val->setText(QString::number(RadiusToDegree(joint_component.GetPosition()), 10, 2));
		}
	);
	joint_component.RegisterDelFunction(ComponentUIFactory::kTag, [w = widget]() {
		w->disconnect();
		}
	);

	// Set Timer as child of Widget make us not conseder about the memory manager 
	QTimer* timer = new QTimer(widget);
	// UI >> Component
	// QObject::connect(slider, &QSlider::sliderMoved, )
	QObject::connect(timer, &QTimer::timeout, [&joint_component, slider]() mutable {
		joint_component.PushPosBuffer(joint_component.GetPosition() + DegreeToRadius(slider->value()));
		});
	QObject::connect(slider, &QSlider::sliderReleased, [slider, timer]() {
		timer->stop();
		slider->setValue(0);
		});

	QObject::connect(slider, &QSlider::sliderPressed, [timer]() {
		timer->start(30);
		});
	QObject::connect(widget, &QWidget::destroyed, [&timer, component = &joint_component]() {		
		component->DeregisterUpdateFunction(ComponentUIFactory::kTag);
		component->DeregisterDelFunction(ComponentUIFactory::kTag);
		});
}

/*______________________Joint Group Component UI BUILDER METHODS______________________________________*/
void CreateJointGroupComponentUI(QWidgetBuilder& builder, int joint_count)
{
	QWidget* widget = builder.GetWidget();
	QIcon left_icon, right_icon, setting_icon;
	left_icon.addFile(QString::fromUtf8(":/icon/leftarrow.png"), QSize(), QIcon::Normal, QIcon::Off);
	right_icon.addFile(QString::fromUtf8(":/icon/rightarrow.png"), QSize(), QIcon::Normal, QIcon::Off);
	setting_icon.addFile(QString::fromUtf8(":/icon/setting.png"), QSize(), QIcon::Normal, QIcon::Off);
	for (int i = 0; i < joint_count; ++i) {
		const QString count_s = QString::number(i + 1);

		QHBoxLayout* single_joint_layout = new QHBoxLayout;
		single_joint_layout->setObjectName(QString::fromUtf8("single_joint_layout") + count_s);
		single_joint_layout->setContentsMargins(0, 0, 0, 0);

		QLabel* joint_tag = new QLabel("Joint\n" + count_s);
		joint_tag->setObjectName(QString::fromUtf8("joint_tag"));
		joint_tag->setMinimumSize(QSize(45, 0));
		QFont font;
		font.setPointSize(11);
		font.setBold(true);
		joint_tag->setFont(font);
		joint_tag->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
		joint_tag->setFrameShape(QFrame::WinPanel);
		joint_tag->setFrameShadow(QFrame::Plain);
		joint_tag->setAlignment(Qt::AlignCenter);

		single_joint_layout->addWidget(joint_tag);

		QVBoxLayout* plugins_layout = new QVBoxLayout;
		plugins_layout->setObjectName(QString::fromUtf8("plugins_layout") + count_s);

		QHBoxLayout* slider_layout = new QHBoxLayout;
		slider_layout->setObjectName(QString::fromUtf8("slider_layout") + count_s);
		
		QToolButton* left_tag = new QToolButton;
		left_tag->setObjectName(QString::fromUtf8("left_tag") + count_s);		
		left_tag->setIcon(left_icon);
		left_tag->setCheckable(false);
		left_tag->setPopupMode(QToolButton::DelayedPopup);
		left_tag->setAutoRaise(false);
		slider_layout->addWidget(left_tag);

		QSlider* joint_slider = new QSlider;
		joint_slider->setObjectName(QString::fromUtf8("joint_slider") + count_s);
		joint_slider->setCursor(QCursor(Qt::SizeHorCursor));
		joint_slider->setMinimum(-1);
		joint_slider->setMaximum(1);
		joint_slider->setOrientation(Qt::Horizontal);
		joint_slider->setStyleSheet(QString::fromUtf8(
			"QSlider::handle:horizontal {\n"
			"    background:  qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(50, 20, 50, 255), stop: 1 rgba(50, 75, 50, 255));\n"			
			"    width: 20px;\n"
			"    margin: -6px; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */\n"
			"    border-radius: 2px;\n"
			"	 border: 1px solid rgb(135,135,135);\n"
			"}\n"
			"\n"
			"QSlider::groove:horizontal {\n"
			"    border: 1px solid rgb(135,135,135);\n"
			"	border-radius: 3px;\n"
			"    height: 5px;\n"
			"    background: qlineargradient(x1:0, y1:0.8, x2:1, y2:1, stop:0 rgb(120,255,120), stop:1 rgb(255,55,80));\n"
			"    margin: -1px 10px;\n"
			"}\n"
			"\n"
			"QSlider::sub-page:horizontal {\n"
			"	border: 1px solid rgb(135,135,135);\n"
			"	border-radius: 3px;\n"
			"    height: 5px;\n"
			"	margin: -1px 0px -1px 10px;\n"
			"    background: qlineargradient(x1:1, y1:0, x2:0, y2:0, stop:0 rgb(120,255,120), stop:1 rgb(80,55,255));\n"
			"}\n"
			"\n"));
		slider_layout->addWidget(joint_slider);

		QToolButton * right_tag = new QToolButton;
		right_tag->setObjectName(QString::fromUtf8("right_tag") + count_s);
		right_tag->setStyleSheet(QString::fromUtf8(""));		
		right_tag->setIcon(right_icon);
		right_tag->setCheckable(true);
		right_tag->setChecked(false);
		right_tag->setAutoRaise(false);
		slider_layout->addWidget(right_tag);

		QHBoxLayout* display_layout = new QHBoxLayout;
		display_layout->setObjectName(QString::fromUtf8("display_layout"));

		QLabel* value_tag = new QLabel("value(deg.)");
		value_tag->setObjectName(QString::fromUtf8("value_tag") + count_s);
		value_tag->setMinimumSize(QSize(75, 0));
		value_tag->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
		value_tag->setStyleSheet(QString::fromUtf8(
			"font: 9pt \"Microsoft YaHei UI\";\n"
			"color: rgb(200, 150, 255);"));
		value_tag->setAlignment(Qt::AlignCenter);

		display_layout->addWidget(value_tag);

		QLineEdit* value = new QLineEdit("0.0");
		value->setObjectName(QString::fromUtf8("value") + count_s);
		value->setStyleSheet(QString::fromUtf8(
			"QLineEdit{\n"
			"font:  9pt \"Cascadia Code\";\n"
			"color: rgb(150, 255, 175);\n"
			"border: 3px double rgb(255, 255, 255);\n"
			"}"));
		value->setAlignment(Qt::AlignCenter);
		display_layout->addWidget(value);

		QToolButton* setting_button = new QToolButton;
		setting_button->setObjectName(QString::fromUtf8("setting_button") + count_s);
		setting_button->setStyleSheet(QString::fromUtf8(
			"QToolButton{\n"
			"color: rgb(255, 255, 255);\n"
			"background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(0, 0, 0, 255), stop:0.2 rgba(220, 195, 255, 155), stop:0.4 rgba(0, 0, 0, 255), stop:0.5 rgba(0, 0, 0, 255), stop:0.8 rgba(155, 155, 255, 100), stop: 1 rgba(100, 180, 100, 150));\n"
			"}"));		
		setting_button->setIcon(setting_icon);
		setting_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
		setting_button->setAutoRaise(false);
		setting_button->setArrowType(Qt::NoArrow);
		display_layout->addWidget(setting_button);

		plugins_layout->addLayout(slider_layout);
		plugins_layout->addLayout(display_layout);

		single_joint_layout->addLayout(plugins_layout);

		builder.AddLayout(single_joint_layout);

	}
	builder.AddSpacer(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
}

void ConnectJointGroupComponentUI(QWidget* widget, JointGroupComponent& component)
{
	const vector<JointComponent*> joints = component.GetJoints();
	int n = component.GetJointsSize();
	for (int i = 0; i < n; ++i) {
		const QString count_s = QString::number(i + 1);

		QHBoxLayout* group_layout = widget->findChild<QHBoxLayout*>(JointLayoutObjName.data()	+ count_s);
		QSlider*     slider		  = widget->findChild<QSlider*>(JointSliderGroupObjName.data()	+ count_s);
		QLineEdit*	 val		  = widget->findChild<QLineEdit*>(JointGroupValObjName.data()	+ count_s);
		QToolButton* setting	  = widget->findChild<QToolButton*>(SettingButtonObjName.data() + count_s);

		// Component >> UI
		component.RegisterUpdateFunction(ComponentUIFactory::kTag, [val, &joint = *joints[i]](float delta){
			val->setText(QString::number(RadiusToDegree(joint.GetPosition()), 10, 2));
		});
		component.RegisterDelFunction(ComponentUIFactory::kTag, [widget]() {
			widget->disconnect();
		});			

		// Set Timer as child of Widget make us not consider about the memory manager 
		QTimer* timer = new QTimer(widget);
		// UI >> Component
		QObject::connect(timer, &QTimer::timeout, [&joint = *joints[i], slider]() mutable {
			joint.PushPosBuffer(joint.GetPosition() + DegreeToRadius(slider->value()));
			});
		QObject::connect(slider, &QSlider::sliderReleased, [slider, timer]() {
			timer->stop();
			slider->setValue(0);
			});

		QObject::connect(slider, &QSlider::sliderPressed, [timer]() {
			timer->start(30);
			});
		QObject::connect(widget, &QWidget::destroyed, [&timer, &component]() {
			component.DeregisterUpdateFunction(ComponentUIFactory::kTag);
			component.DeregisterDelFunction(ComponentUIFactory::kTag);
			});
	}
}

/*__________________________Tracker Component UI BUILDER METHODS______________________________________*/
void ConnectTrackerComponentUI(TrackerComponentWidget* widget, TrackerComponent& component)
{
	auto string_to_QString = [](const string& val)->QString {
		return QString::fromStdString(val);
	};
	list<string> trackables = component.GetTrackableNames();
	QStringList q_trackable_list(trackables.size());
	std::transform(std::execution::par_unseq, 
		trackables.begin(), trackables.end(), q_trackable_list.begin(), string_to_QString);
	widget->AddTrackables(q_trackable_list);

	const list<string>& tracers = component.GetTracerNames();
	QStringList q_tracer_list(tracers.size());
	std::transform(std::execution::par_unseq,
		tracers.begin(), tracers.end(), q_tracer_list.begin(), string_to_QString);
	widget->AddTracers(q_tracer_list);

	widget->SetGoal(QString::fromStdString(component.GetGoal()));

	widget->SetButtonChecked(static_cast<int>(component.GetState()));

	// Component >> UI
	component.RegisterDelFunction(ComponentUIFactory::kTag, [widget]() {
		widget->disconnect();
		});

	// Component << UI
	QObject::connect(widget, &TrackerComponentWidget::StateChangedRequest, [&tracker = component](int index) {
		tracker.SetState(static_cast<TrackerComponent::State>(index));
		});
	QObject::connect(widget, &TrackerComponentWidget::TragetSelectedRequest, [&tracker = component](const QString& goal_name) {
		tracker.SetGoal(goal_name.toStdString());
		});
	QObject::connect(widget, &TrackerComponentWidget::destroyed, [&component]() {
		component.DeregisterUpdateFunction(ComponentUIFactory::kTag);
		component.DeregisterDelFunction(ComponentUIFactory::kTag);
		});
}

/*__________________________Kinematic Component UI BUILDER METHODS______________________________________*/
void ConnectKinematicComponentUI(KinematicComponentWidget* widget, KinematicComponent& component)
{
	widget->SetJointCount(component.GetJointCount());
	widget->SetIKSolver(static_cast<int>(component.GetIKEnum()));
	widget->SetMaxIterations(component.GetMaxIteration());
	widget->SetStepDecay(component.GetDecayScaler());
	widget->SetPrecision(component.GetPrecision());
	
	// TODO: complete the rest connect
	// Component >> UI
	component.RegisterDelFunction(ComponentUIFactory::kTag, [widget]() {
		widget->disconnect();
		});
	
	// Component << UI
	QObject::connect(widget, &KinematicComponentWidget::IKSolverSwitchRequest, [&component](int idx) {
		component.SetIKSolver(static_cast<IKSolverEnum>(idx));
		});
	QObject::connect(widget, &KinematicComponentWidget::destroyed, [&component]() {
		component.DeregisterUpdateFunction(ComponentUIFactory::kTag);
		component.DeregisterDelFunction(ComponentUIFactory::kTag);
		});
}

/*__________________________Material Component UI BUILDER METHODS______________________________________*/
const std::string ComponentUIFactory::kTag = "UI";

QWidget* ComponentUIFactory::Create(Component& component)
{
	string_view s = component.GetTypeName();
	QWidgetBuilder builder;

	// set delete functions for component and widget both
	auto set_del_function = [&com = component](QWidget* widget) {
		com.RegisterDelFunction(ComponentUIFactory::kTag, [widget]() { widget->disconnect(); });
		QObject::connect(widget, &QWidget::destroyed, [&com]() { 
			com.DeregisterDelFunction(ComponentUIFactory::kTag);
			com.DeregisterUpdateFunction(ComponentUIFactory::kTag);
		});		
	};
	if (s == "TransformComponent") {
		TransformCom& com = dynamic_cast<TransformCom&>(component);
		QWidget*     widget = new QWidget;
		QVBoxLayout* layout = new QVBoxLayout;
		
		constexpr const float kMinTrans = std::numeric_limits<float>::lowest(),
							  kMaxTrans = std::numeric_limits<float>::max();
		QLayout* trans_layout = CreateVector3Editor("trans(m.)", 
							std::bind(&TransformCom::GetTransGlobal, &com),
							std::bind(&TransformCom::SetTransGlobal, &com, std::placeholders::_1, true),
							kMinTrans, kMaxTrans, 0.01f);
		Vector3Edit* trans_edit = reinterpret_cast<Vector3Edit*>(trans_layout->itemAt(1)->widget());
		com.RegisterUpdateFunction(ComponentUIFactory::kTag, [edit = trans_edit, &com]
		(float delta) {
			edit->SetValue(QConversion::fromVec3f(com.GetTransGlobal()));
		});
		layout->addLayout(trans_layout);

		constexpr const float kPi = 3.141592653522535f;
		QLayout* rot_layout = CreateVector3Editor("rotation(rad.)", 
							[&com]()->Eigen::Vector3f{ return ToXYZEuler(com.GetRotGlobal());},
							[&com](const Eigen::Vector3f& vec)->void { com.SetRotGlobal(FromXYZEuler(vec), true);},
							-99.9, 99.9, 0.01f);
		Vector3Edit* rot_edit = reinterpret_cast<Vector3Edit*>(rot_layout->itemAt(1)->widget());
		com.RegisterUpdateFunction(ComponentUIFactory::kTag, [edit = rot_edit, &com]
		(float delta) {
			if (com.GetIsDirty()) {
				edit->SetValue(QConversion::fromVec3f(ToXYZEuler(com.GetRotGlobal())));
			}
		});
		layout->addLayout(rot_layout);
		
		QLayout* scl_layout = CreateVector3Editor("scale", 
							std::bind(&TransformCom::GetScale, &com),
							std::bind(&TransformCom::SetScale, &com, std::placeholders::_1, true),
							0.0f, std::numeric_limits<float>::max(), 0.01f);
		Vector3Edit* scl_edit = reinterpret_cast<Vector3Edit*>(scl_layout->itemAt(1)->widget());
		com.RegisterUpdateFunction(ComponentUIFactory::kTag, [edit = scl_edit, &com]
		(float delta) { 
			edit->SetValue(QConversion::fromVec3f(com.GetScale()));
		});
		layout->addLayout(scl_layout);

		layout->addSpacerItem(new QSpacerItem(40, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));
		widget->setLayout(layout);
		
		set_del_function(widget);
		return   widget;
	}
	else if (s == "JointComponent") {
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
		auto add_property_ui = [](auto&& com, auto&& layout) {
			for (auto& pro : com.GetProperties()) {
				QLayout* new_layout = build_map[pro.type](pro.name, pro.val);
				if (new_layout) {
					layout->addLayout(new_layout);
				}
			}
			layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding));
		};
		
		QWidget*     widget	= new QWidget;
		QVBoxLayout* layout = new QVBoxLayout;
		
		layout->setSpacing(2);
		layout->addWidget(CreateStandardTextLabel("shader"));
		
		auto shader_names = ResourceManager::getInstance().GetShadersName();
		QStringList names_list; names_list.reserve(shader_names.size());
		for (auto& name : shader_names) names_list.append(QString::fromStdString(name));		
		GSelector* selector = new GSelector(nullptr, QString::fromStdString(material_component.GetShader()), names_list);	
		selector->setMinimumHeight(kEleMiniHeight);
		QObject::connect(selector, &GSelector::textChanged, 
			[&com = material_component, layout = layout, ui_func = add_property_ui]
			(const QString& shader_name) {
				
				com.SetShader(shader_name.toStdString());
				auto clear_layout = [](auto&& clear_layout, QLayout* layout, int remain)->void{
					int item_count = layout->count();
					while (item_count > remain) {
						auto item = layout->itemAt(item_count - 1);
						if (QWidget* widget = item->widget()) {
							layout->removeWidget(widget);
							widget->deleteLater();
						}
						else if (QLayout* child_layout = item->layout()) {
							clear_layout(clear_layout, child_layout, 0);
							delete child_layout;
						}
						else if (QSpacerItem* spacer = item->spacerItem()) {
							layout->removeItem(spacer);
							delete spacer;
						}						
						--item_count;
					}
				};
				clear_layout(clear_layout, layout, 4);
				ui_func(com, layout);
		});
		layout->addWidget(selector);

		layout->addWidget(CreateStandardTextLabel("cast shadow"));
		layout->addLayout(CreateStanardCheckBox(std::bind(&MaterialComponent::GetIsCastShadow, &material_component), 
												std::bind(&MaterialComponent::SetIsCastShadow, &material_component, std::placeholders::_1)));

		add_property_ui(material_component, layout);
		
		widget->setLayout(layout);
		widget->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Expanding);
		
		set_del_function(widget);
		return widget;
	}
	else if (s == "ColliderComponent") {
		ColliderComponent& collider_component = dynamic_cast<ColliderComponent&>(component);
		auto shapes = collider_component.GetShapes();

		QWidget*	 widget = new QWidget;
		QVBoxLayout* layout = new QVBoxLayout;

		layout->addWidget(CreateStandardTextLabel("static"));
		layout->addLayout(CreateStanardCheckBox(std::bind(&ColliderComponent::IsStatic,  &collider_component),
												std::bind(&ColliderComponent::SetStatic, &collider_component, std::placeholders::_1)));
		layout->addWidget(CreateStandardTextLabel("group"));
		QSpinBox* spinbox = new QSpinBox;
		spinbox->setValue(collider_component.GetGroup());
		QObject::connect(spinbox, &QSpinBox::valueChanged, 
			[&com = collider_component](int value) { 
				com.SetGroup(value); 
			});
		spinbox->setMinimum(0);
		spinbox->setSingleStep(1);
		layout ->addWidget(spinbox);

		QComboBox*   selector    = new QComboBox(widget);
		selector->setStyleSheet(combo_editor_qss.data());
		QStringList  lists(shapes.size(), "shape");
		for (int i = 1; auto & s : lists) s.append("_" + QString::number(i++));
		selector->addItems(lists);		
		layout->addWidget(selector);
		

		QStackedWidget* stacked = new QStackedWidget;
		for (int count = 0; auto & shape : shapes) {
			QWidget* sub_widget = new QWidget;
			QLayout* sub_layout = shapes_builder_map[shape->GetShapeType()](shape);
			
			QPushButton* del_button = new QPushButton("Del");
			del_button->setStyleSheet(normal_button_qss.data());
			QObject::connect(del_button, &QPushButton::clicked, [&com = collider_component, stacked = stacked, selector = selector]() {
				int idx = stacked->currentIndex();
				com.DeregisterShape(com.GetShape(idx));
				stacked->removeWidget(stacked->widget(idx));
				selector->removeItem(idx);
			});
			sub_layout->addWidget(del_button);
			sub_layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Ignored, QSizePolicy::Expanding));
			sub_widget->setLayout(sub_layout);
			stacked->addWidget(sub_widget);

			++count;
		}
		layout->addWidget(stacked);

		// TODO: add a inteface that when ColliderComponent is Changed the ui can modify
		QObject::connect(selector, &QComboBox::activated, stacked, &QStackedWidget::setCurrentIndex);

		widget->setLayout(layout);
		widget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);

		set_del_function(widget);
		return widget;
	}
	else {
		// TODO: add the widget
		QWidget* widget = new QWidget;
		return   widget;
	}

	return nullptr;
}

}


