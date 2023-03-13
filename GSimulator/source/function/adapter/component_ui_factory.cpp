#include "function/adapter/component_ui_factory.h"

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

#include "ColorEdit/color_edit.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtWidgets/QDial>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLayout>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QHBoxLayout>

#include <QtGui/QDropEvent>
#include <QtGui/QDragEnterEvent>

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

// ui Create table acoording the class with Shader Properties
std::unordered_map<std::string, std::function<QLayout*(std::string, ShaderProperty::Var&)>> ComponentUIFactory::build_map = {
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
		
		layout->addWidget(CreateStandardTextLabel(name));

		QDoubleSpinBox* spinbox = new QDoubleSpinBox;		
		spinbox->setValue(std::get<float>(val));
		spinbox->setStyleSheet(double_spin_box_qss.data());
		QObject::connect(spinbox, &QDoubleSpinBox::valueChanged, [&val = val](double value) { val = static_cast<float>(value); });
		layout->addWidget(spinbox);
		
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
	joint_component.RegisterUpdateFunction([val, &joint_component](float delta) {
		val->setText(QString::number(RadiusToDegree(joint_component.GetPosition()), 10, 2));
		}
	);
	joint_component.RegisterDelFunction([w = widget]() {
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
		component->DeregisterUpdateFunction();
		component->DeregisterDelFunction();
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
		component.RegisterUpdateFunction([val, &joint = *joints[i]](float delta){
			val->setText(QString::number(RadiusToDegree(joint.GetPosition()), 10, 2));
		});
		component.RegisterDelFunction([widget]() {
			widget->disconnect();
		});			

		// Set Timer as child of Widget make us not conseder about the memory manager 
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
			component.DeregisterUpdateFunction();
			component.DeregisterDelFunction();
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
	component.RegisterDelFunction([widget]() {
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
		component.DeregisterUpdateFunction();
		component.DeregisterDelFunction();
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
	component.RegisterDelFunction([widget]() {
		widget->disconnect();
		});
	
	// Component << UI
	QObject::connect(widget, &KinematicComponentWidget::IKSolverSwitchRequest, [&component](int idx) {
		component.SetIKSolver(static_cast<IKSolverEnum>(idx));
		});
	QObject::connect(widget, &KinematicComponentWidget::destroyed, [&component]() {
		component.DeregisterUpdateFunction();
		component.DeregisterDelFunction();
		});
}

/*__________________________Material Component UI BUILDER METHODS______________________________________*/


QWidget* ComponentUIFactory::Create(Component& component)
{
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
		auto& properties = material_component.GetProperties();

		QWidget* widget = new QWidget;
		QVBoxLayout* layout = new QVBoxLayout;
		layout->setSpacing(2);

		layout->addWidget(CreateStandardTextLabel("shader"));
		
		QLineEdit* shader_editor = new DragAcceptorEditor(QString::fromStdString(material_component.GetShader()), 
														  nullptr);
		shader_editor->setMinimumHeight(kEleMiniHeight);
		layout->addWidget(shader_editor);

		layout->addWidget(CreateStandardTextLabel("cast shadow"));
		layout->addLayout(CreateStanardCheckBox(std::bind(&MaterialComponent::GetIsCastShadow, &material_component), 
												std::bind(&MaterialComponent::SetIsCastShadow, &material_component, std::placeholders::_1)));

		for (auto&& pro : properties) {
			layout->addLayout(build_map[pro.type](pro.name, pro.val));
		}

		layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding));
		widget->setLayout(layout);
		widget->setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Expanding);
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


