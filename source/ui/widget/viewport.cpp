#include "viewport.h"

#include "manager/resourcemanager.h"
#include "manager/modelmanager.h"
#include "manager/rendermanager.h"
#include "manager/physicsmanager.h"
#include "manager/planningmanager.h"
#include "manager/tcpsocketmanager.h"

#include <QtGui/QMouseEvent>
#include <QtGUI/QKeyEvent>
#include <QtCore/QmetaType>

#ifdef _DEBUG
#include <iostream>
#include <format>
#endif
#include "component/collider_component.h"

#include <GComponent/Geometry/gcollision_detection.h>

namespace GComponent {

Viewport::Viewport(QWidget* parent) :
	QOpenGLWidget(parent),
	ui_state_(width(), height()),
	gl_(std::make_shared<GComponent::MyGL>())
{
	qRegisterMetaType<Viewport>("viewport");
	setFocusPolicy(Qt::StrongFocus);
	QSurfaceFormat set_format;
	set_format.setSamples(4);
	set_format.setVersion(4, 5);
	setFormat(set_format);

	std::cout << std::format("the sample num : {}\n", format().samples());
}

Viewport::~Viewport() {}


void Viewport::initializeGL()
{
	gl_->initializeOpenGLFunctions();
	
	RegisteredShader();
	if (!camera_handle)
		camera_handle = GComponent::ModelManager::getInstance().RegisteredCamera();
	
	ui_state_.SetGL(gl_);	
	GComponent::ResourceManager::getInstance().SetGL(gl_);
	GComponent::RenderManager::getInstance().SetGL(gl_);

	QOpenGLContext::currentContext()->format().setSwapInterval(0);
}

void Viewport::resizeGL(int w, int h)
{
	//gl_->glViewport(0.0f, 0.0f, w, h);
	ui_state_.OnResize(w, h);
}

void Viewport::paintGL()
{
	using namespace glm;
	using namespace GComponent;
	static std::chrono::time_point last_point = std::chrono::steady_clock::now();
	static float delta = 0.0f;
	
	/*_________________________________Paint Main Loop________________________________________________________________________________________________*/
	Camera* camera_ptr = ModelManager::getInstance().GetCameraByHandle(camera_handle);
	// Set global Render parameters
	RenderGlobalInfo& render_info = RenderManager::getInstance().m_render_sharing_msg;
	render_info.SetSimpleDirLight(vec3(0.5f, 1.0f, 1.0f), vec3(1.0f));
	render_info.SetCameraInfo(*camera_ptr);
	render_info.SetProjectionPlane(0.001f, 1000.0f);
	render_info.UpdateProjectionMatrix();
	
	// custom definition update method
	CustomUpdateImpl();

	// Process Input
	ui_state_.tick();
	
	// Adjust the planning
	PlanningManager::getInstance().tick(delta_time.count());
	// Sync the frame
	TcpSocketManager::getInstance().tick();
	// Adjust all component
	ModelManager::getInstance().tickAll(delta_time.count());
	// Adjust all resources
	ResourceManager::getInstance().tick(gl_);
	// Adjust all the physics actors
	PhysicsManager::getInstance().tick(delta_time.count());
	// Draw all renderable process Passes
	RenderManager::getInstance().tick();
	
	// 
	// Time statics rendering over
	std::chrono::time_point now = std::chrono::steady_clock::now();
	delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_point);
	last_point = now;
	emit EmitDeltaTime(delta_time.count());
	update();
}

void Viewport::CustomUpdateImpl()
{
	Model* cube0 = ModelManager::getInstance().GetModelByName("cube0");
	Model* cube1 = ModelManager::getInstance().GetModelByName("cube1");
	Model* sphere0 = ModelManager::getInstance().GetModelByName("sphere0");
	Model* capsule0 = ModelManager::getInstance().GetModelByName("capsule0");
	
	auto col_com0 = cube0->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
	auto col_com1 = cube1->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
	auto col_com2 = sphere0->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());
	auto cap_com3 = capsule0->GetComponent<ColliderComponent>(ColliderComponent::type_name.data());	

	auto shapes0 = col_com0->GetShapes();
	auto shapes1 = col_com1->GetShapes();
	auto shapes2 = col_com2->GetShapes();
	auto shape_cap0 = cap_com3->GetShapes();

	cube0->intesection_ = cube1->intesection_ 
						= sphere0->intesection_
						= capsule0->intesection_
						= false;

	for (auto& s0 : shapes0) {
		auto box0 = dynamic_cast<BoxShape*>(s0);
		for (auto& s1 : shapes1) {
			auto box1 = dynamic_cast<BoxShape*>(s1);
			if (IntersectOBBOBB(Vec3(box0->m_half_x, box0->m_half_y, box0->m_half_z), cube0->getTransGlobal(), cube0->getRotGlobal(),
				Vec3(box1->m_half_x, box1->m_half_y, box1->m_half_z), cube1->getTransGlobal(), cube1->getRotGlobal())) {
				cube0->intesection_ = true;
				cube1->intesection_ = true;
				goto boxcheckfinish;
			}
		}
	}

boxcheckfinish:
	for (auto& s0 : shapes0) {
		auto box0 = dynamic_cast<BoxShape*>(s0);
		for (auto& s1 : shapes2) {
			auto sshape = dynamic_cast<SphereShape*>(s1);
			if (IntersectOBBSphere(Vec3(box0->m_half_x, box0->m_half_y, box0->m_half_z), cube0->getTransGlobal(), cube0->getRotGlobal(),
				sshape->m_radius, sphere0->getTransGlobal())) {
				cube0->intesection_   = true;
				sphere0->intesection_ = true;				
			}
		}
	}

	for (auto& s0 : shapes1) {
		auto box0 = dynamic_cast<BoxShape*>(s0);
		for (auto& s1 : shapes2) {
			auto sshape = dynamic_cast<SphereShape*>(s1);
			if (IntersectOBBSphere(Vec3(box0->m_half_x, box0->m_half_y, box0->m_half_z), cube1->getTransGlobal(), cube1->getRotGlobal(),
				sshape->m_radius, sphere0->getTransGlobal())) {
				cube1->intesection_   = true;
				sphere0->intesection_ = true;				
			}
		}
	}

	for (auto& s0 : shape_cap0) {
		auto shape_cap0 = dynamic_cast<CapsuleShape*>(s0);
		for (auto& s1 : shapes0) {
			auto box_shape = dynamic_cast<BoxShape*>(s1);
			if (IntersectOBBCaspsule(Vec3(box_shape->m_half_x, box_shape->m_half_y, box_shape->m_half_z), cube0->getTransGlobal(), cube0->getRotGlobal(),
				shape_cap0->m_radius, shape_cap0->m_half_height, capsule0->getTransGlobal(), capsule0->getRotGlobal())) {
				cube0->intesection_ = true;
				capsule0->intesection_ = true;
			}
		}
	}
}

/*________________________________Events Implementations_____________________________________________*/
void Viewport::keyPressEvent(QKeyEvent* event)
{
	GComponent::Camera* camera_ptr = GComponent::ModelManager::getInstance().GetCameraByHandle(camera_handle);
	size_t key_state = 0x0;
	if (event->key() == Qt::Key_W) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyW);
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::FORWARD, delta_time.count());
	}
	if (event->key() == Qt::Key_S) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyS);
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::BACKWARD, delta_time.count());
	}
	if (event->key() == Qt::Key_A) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyA);
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::LEFT, delta_time.count());
	}
	if (event->key() == Qt::Key_D) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyD);
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::RIGHT, delta_time.count());
	}
	if (event->key() == Qt::Key_Q) {
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::UP, delta_time.count());
	}
	if (event->key() == Qt::Key_E) {
		camera_ptr->ProcessKeyMovementCommand(GComponent::CameraMoveMent::DOWN, delta_time.count());
	}

	if (event->key() == Qt::Key_Delete) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyDelete);
		//std::cout << std::set_format("we would like to current Selected Model: {}\n", 0);
	}

	ui_state_.OnKeyPress(key_state);
}

void Viewport::keyReleaseEvent(QKeyEvent* event)
{
	size_t key_state = 0x0;
	if (event->key() == Qt::Key_W) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyW);
	}
	if (event->key() == Qt::Key_S) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyS);
	}
	if (event->key() == Qt::Key_A) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyA);
	}
	if (event->key() == Qt::Key_D) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyD);
	}
	if (event->key() == Qt::Key_Delete) {
		key_state |= static_cast<size_t>(GComponent::KeyButtonState::KeyDelete);
	}

	ui_state_.OnKeyRelease(key_state);
}

void Viewport::mouseMoveEvent(QMouseEvent* event)
{
	ui_state_.OnCursorMove(event->x(), event->y());
	if (event->buttons() &= Qt::MouseButton::RightButton)
	{
		GComponent::Camera* camera_ptr = GComponent::ModelManager::getInstance().GetCameraByHandle(camera_handle);
		QPoint cur_point = event->pos();
		camera_ptr->ProcessMouseMovement(cur_point.x() - mouse_right_pressed.x(), -cur_point.y() + mouse_right_pressed.y());
		mouse_right_pressed = cur_point;
	}
}

void Viewport::mousePressEvent(QMouseEvent* event)
{
	ui_state_.OnMousePress(event->buttons());
	if (event->button() == Qt::MouseButton::RightButton)
	{
		mouse_right_pressed = event->pos();
	}
}

void Viewport::mouseReleaseEvent(QMouseEvent* event)
{
	ui_state_.OnMouseRelease(event->button());
}

void Viewport::enterEvent(QEnterEvent* event)
{
	ui_state_.OnMouseEnter(event->x(), event->y());
	setMouseTracking(true);
}

void Viewport::leaveEvent(QEvent* event)
{
	ui_state_.OnMouseLeave();
	setMouseTracking(false);
}

void Viewport::RegisteredShader()
{
	GComponent::ResourceManager::getInstance().RegisteredShader("skybox",		new GComponent::MyShader(nullptr, PathVert(skybox), PathFrag(skybox)));
	GComponent::ResourceManager::getInstance().RegisteredShader("postprocess",	new GComponent::MyShader(nullptr, PathVert(postprocess), PathFrag(postprocess)));
	GComponent::ResourceManager::getInstance().RegisteredShader("color",		new GComponent::MyShader(nullptr, PathVert(Color),      PathFrag(Color)));
    GComponent::ResourceManager::getInstance().RegisteredShader("axis",			new GComponent::MyShader(nullptr, PathVert(axis),       PathFrag(axis)));
    GComponent::ResourceManager::getInstance().RegisteredShader("picking",		new GComponent::MyShader(nullptr, PathVert(picking),    PathFrag(picking)));
    GComponent::ResourceManager::getInstance().RegisteredShader("base",			new GComponent::MyShader(nullptr, PathVert(Base),       PathFrag(Base)));
    GComponent::ResourceManager::getInstance().RegisteredShader("linecolor",	new GComponent::MyShader(nullptr, PathVert(LineColor),  PathFrag(LineColor)));
}

}