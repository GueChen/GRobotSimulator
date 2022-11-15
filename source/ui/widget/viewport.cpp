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
#include "component/rigidbody_component.h"

namespace GComponent {

Viewport::Viewport(QWidget* parent) :
	QOpenGLWidget(parent),
	ui_state_(width(), height()),
	gl_(std::make_shared<GComponent::MyGL>())
{
	qRegisterMetaType<Viewport>("viewport");
	setFocusPolicy(Qt::StrongFocus);
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
	
	//Model* sphere_collider = ModelManager::getInstance().GetModelByName("sphere0");
	//if (sphere_collider) {
	//	
	//	float time_point = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::steady_clock::now().time_since_epoch()).count();
	//	/*sphere_collider->setTransLocal(Eigen::Vector3f(
	//							0.25f + 0.3f   * sin(0.5f * time_point), 
	//							0.85f + 0.125f * cos(0.5f * 0.33f * time_point), 
	//							1.75f + 0.2f   * sin(0.5f * 3.714f * time_point)));*/
	//	std::shared_ptr<PhysicsScene> scene = PhysicsManager::getInstance().GetActivateScene().lock();
	//	auto rigid_com = sphere_collider->GetComponent<RigidbodyComponent>(RigidbodyComponent::type_name.data());
	//	vector<OverlapHitInfo> hits_info;
	//	scene->Overlap(rigid_com->GetActor(), 10, hits_info);
	//	for (auto&& [hitter, vec] : hits_info) {
	//		std::cout << sphere_collider->getName() << " collide with " << 
	//			PhysicsManager::getInstance().GetModelIdByActorID(hitter)->GetParent()->getName() <<
	//			"\npenetrac vec: " << vec.transpose() << std::endl;
	//	}
	//}
	
	
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
		//std::cout << std::format("we would like to current Selected Model: {}\n", 0);
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