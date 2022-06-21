#include "viewport.h"

#include "manager/resourcemanager.h"
#include "manager/modelmanager.h"
#include "manager/rendermanager.h"

#include <QtGui/QMouseEvent>
#include <QtGUI/QKeyEvent>
#include <QtCore/QmetaType>

#include <iostream>
#include <format>

namespace GComponent {

Viewport::Viewport(QWidget* parent) :
	QOpenGLWidget(parent),
	ui_state_(width(), height()),
	gl_(std::make_shared<GComponent::MyGL>()),
	grid_(std::make_unique<BaseGrid>(50, 0.1f)),
	sky_box_(new SkyBox)
{
	qRegisterMetaType<Viewport>("viewport");
	setFocusPolicy(Qt::StrongFocus);
	ModelManager::getInstance().RegisteredAuxiModel("skybox", sky_box_);
}

Viewport::~Viewport() {}

void Viewport::initializeGL()
{
	gl_->initializeOpenGLFunctions();

	RegisteredShader();
	if (!camera_handle)
		camera_handle = GComponent::ModelManager::getInstance().RegisteredCamera();
	GComponent::ResourceManager::getInstance().DeregisteredUIHandle("viewport");
	
	grid_->setGL(gl_);
	ui_state_.SetGL(gl_);
	GComponent::ModelManager::getInstance().SetGL(gl_);
	GComponent::ResourceManager::getInstance().SetGL(gl_);
	GComponent::RenderManager::getInstance().SetGL(gl_);

	GComponent::ResourceManager::getInstance().RegisteredUIHandle("viewport", this);
	QOpenGLContext::currentContext()->format().setSwapInterval(0);
}

void Viewport::resizeGL(int w, int h)
{
	gl_->glViewport(0.0f, 0.0f, w, h);
	ui_state_.OnResize(w, h);
}

void Viewport::paintGL()
{
	using namespace glm;
	using namespace GComponent;
	using enum AxisSelected;

	static std::chrono::time_point last_point = std::chrono::steady_clock::now();
	static float delta = 0.0f;

	Camera* camera_ptr = ModelManager::getInstance().GetCameraByHandle(camera_handle);
	// Process Input
	ui_state_.tick();
	sky_box_->tick(delta);
	// Set global parameters
	ModelManager::getInstance().SetProjectViewMatrices(
		perspective(radians(camera_ptr->Zoom), static_cast<float>(width()) / height(), 0.1f, 1000.0f),
		camera_ptr->GetViewMatrix());
	ModelManager::getInstance().SetDirLightViewPosition(vec3(0.5f, 1.0f, 1.0f), vec3(1.0f), camera_ptr->Position);
	// Adjust all component
	ModelManager::getInstance().tickAll(delta_time.count());
	// Adjust all resources
	ResourceManager::getInstance().tick(gl_);
	// Draw all renderable process Passes
	RenderManager::getInstance().tick();

	MyShader* shader = ResourceManager::getInstance().GetShaderByName("base");
	shader->use();
	shader->setMat4("model", glm::mat4(1.0f));
	shader->setBool("colorReverse", false);
	grid_->Draw();

	

	std::chrono::time_point now = std::chrono::steady_clock::now();
	delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_point);
	last_point = now;
	emit EmitDeltaTime(delta_time.count());
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

#ifdef  _DEBUG
	static std::chrono::time_point last_point = std::chrono::steady_clock::now();
	static float input_delta_time = 0.0f;
	std::chrono::time_point now = std::chrono::steady_clock::now();
	input_delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_point).count();
	last_point = now;
	std::cout << "Input delta time: " << input_delta_time << std::endl;
	std::cout << event->text().toStdString() << std::endl;
#endif //  _DEBUG

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
	GComponent::ResourceManager::getInstance().RegisteredShader("skybox",	 new GComponent::MyShader(nullptr, PathVert(skybox),	 PathFrag(skybox)));
	GComponent::ResourceManager::getInstance().RegisteredShader("color",     new GComponent::MyShader(nullptr, PathVert(Color),      PathFrag(Color)));
    GComponent::ResourceManager::getInstance().RegisteredShader("axis",      new GComponent::MyShader(nullptr, PathVert(axis),       PathFrag(axis)));
    GComponent::ResourceManager::getInstance().RegisteredShader("picking",   new GComponent::MyShader(nullptr, PathVert(picking),    PathFrag(picking)));
    GComponent::ResourceManager::getInstance().RegisteredShader("base",      new GComponent::MyShader(nullptr, PathVert(Base),       PathFrag(Base)));
    GComponent::ResourceManager::getInstance().RegisteredShader("linecolor", new GComponent::MyShader(nullptr, PathVert(LineColor),  PathFrag(LineColor)));
}

}