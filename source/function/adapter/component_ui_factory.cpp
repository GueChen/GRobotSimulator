#include "function/adapter/component_ui_factory.h"

namespace GComponent {
using std::string;

/*______________________Joint Component UI BUILDER METHODS__________________________________________*/
void ComponentUIFactory::CreateJointComponentUI(QWidgetBuilder& builder) {
	{
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
}

void ComponentUIFactory::ConnectJointComponentUI(QWidget* widget, JointComponent& joint_component)
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
void GComponent::ComponentUIFactory::CreateJointGroupComponentUI(QWidgetBuilder& builder, int joint_count)
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

void ComponentUIFactory::ConnectJointGroupComponentUI(QWidget* widget, JointGroupComponent& component)
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
void GComponent::ComponentUIFactory::ConnectTrackerComponentUI(TrackerComponentWidget* widget, TrackerComponent& component)
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
void GComponent::ComponentUIFactory::ConnectKinematicComponentUI(KinematicComponentWidget* widget, KinematicComponent& component)
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

}


