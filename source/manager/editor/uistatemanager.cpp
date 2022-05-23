/**
 *  @file  	ui_state.cpp
 *  @brief 	This class process all state Pass through UI, this file is implementations.
 *  @author Gue Chen<guechen@buaa.edu.cn>
 *  @date 	May 7th, 2022
 **/
#include "manager/editor/uistatemanager.h"

#include "manager/rendermanager.h"
#include "manager/modelmanager.h"

#include <exception>
#ifdef _DEBUG
#include <iostream>
#include <format>
#endif // !

GComponent::UIState::UIState(unsigned w, unsigned h, int segments, float radius):
	m_width(w), m_height(h), m_aspect(static_cast<float>(m_width)/ m_height)
{
	m_translation_axis	= new QtGLTranslationAxis;
	m_scale_axis		= new QtGLScaleAxis;
	m_rotation_axis		= new QtGLRotationAxis;

	ModelManager::getInstance().RegisteredAuxiModel(m_translation_axis->getName(), m_translation_axis);
	ModelManager::getInstance().RegisteredAuxiModel(m_rotation_axis->getName(),	   m_rotation_axis);
	ModelManager::getInstance().RegisteredAuxiModel(m_scale_axis->getName(),       m_scale_axis);

	Init(segments, radius);
}

GComponent::UIState::~UIState()
{
	m_cur_axis = nullptr;
}

void GComponent::UIState::tick()
{
	ProcessDelete();

	if (is_enter_area) 
	{
		// picking process
		picking_msg_ = GetPickingPixelInfo();
		RenderManager::getInstance().SetPickingController(picking_controller);

		// mouse pos process
		m_mouse_delta_x		= m_mouse_pos_x - m_last_mouse_pos_x;
		m_mouse_delta_y		= m_mouse_pos_y - m_last_mouse_pos_y;
		m_last_mouse_pos_x	= m_mouse_pos_x;
		m_last_mouse_pos_y	= m_mouse_pos_y;
	}
	else 
	{
		// picking process
		if (selected_id_buffer != BufferNoValue) 
		{
			selected_id		   = selected_id_buffer;
			selected_id_buffer = BufferNoValue;
		}
		picking_msg_ = std::nullopt;

		// mouse pos process
		m_mouse_pos_x	= m_last_mouse_pos_x = -1;
		m_mouse_pos_y	= m_last_mouse_pos_y = -1;
		m_mouse_delta_x = 0;
		m_mouse_delta_y = 0;
	}

	if (selected_id != NoneSelected) 
	{		
		switch (m_axis_mode) 
		{
			using enum AxisMode;
			case Translation:	m_cur_axis = m_translation_axis;break;
			case Rotation:		m_cur_axis = m_rotation_axis;	break;
			case Scale:			m_cur_axis = m_scale_axis;		break;	
			default:			throw std::exception("The AxisMode Not Define!");
		}
	}

	if (is_draged && m_axis_selected != AxisSelected::None) {
		Camera* camera = ModelManager::getInstance().GetCameraByHandle(1);
		mat4 inv_view = inverse(camera->GetViewMatrix()), inv_proj = inverse(camera->GetProjection(m_aspect));
		mat4 inv_view_rot_only = mat4(inv_view[0][0], inv_view[0][1], inv_view[0][2],			0.0f,
									  inv_view[1][0], inv_view[1][1], inv_view[1][2],			0.0f,
									  inv_view[2][0], inv_view[2][1], inv_view[2][2],			0.0f,
												0.0f,			0.0f,			0.0f,			1.0f);
		
		vec4 screen_uv_coord(static_cast<float>(m_mouse_delta_x)   / m_width, 
							 static_cast<float>(-m_mouse_delta_y) / m_height, 0.0f, 1.0f);
		vec4 line_in_eye   = vec4(vec2(inv_proj * screen_uv_coord), 0.0f, 1.0f);
		vec4 line_in_world = inv_view_rot_only * line_in_eye;

		vec3 dir;
		switch (m_axis_selected) {
			using enum AxisSelected;
			case xAxis:	dir = vec3(1.0f, 0.0f, 0.0f); break;
			case yAxis: dir = vec3(0.0f, 1.0f, 0.0f); break;
			case zAxis: dir = vec3(0.0f, 0.0f, 1.0f); break;
		}	
		dir = (dir.x * line_in_world.x + dir.y * line_in_world.y + dir.z * line_in_world.z) * dir;
		if (Model* selected_obj = ModelManager::getInstance().GetModelByHandle(selected_id);
			selected_obj) {
			switch (m_axis_mode) { using enum AxisMode;
			case Translation: {
				Eigen::Matrix4f global_trans    = Conversion::toMat4f(selected_obj->getTranslationModelMatrix());
				global_trans.block(0, 3, 3, 1) += Conversion::toVec3f(dir);
				Eigen::Matrix4f local_trans     = InverseSE3(Conversion::toMat4f(selected_obj->getParentModelMatrix())) * global_trans;			
				selected_obj->setTransLocal(Conversion::fromVec3f(local_trans.block(0, 3, 3, 1)));
				break;
			}
			case Rotation: {			
				Eigen::Matrix3f global_rot_dcm  = Conversion::toMat4f(selected_obj->getModelMatrix()).block(0, 0, 3, 3);
				Eigen::Matrix3f local_rot_dcm   = Conversion::toMat4f(selected_obj->getParentModelMatrix()).block(0, 0, 3, 3).transpose() * Roderigues(Conversion::toVec3f(dir)) * global_rot_dcm;			
				selected_obj->setRotLocal(Conversion::fromVec3f(LogMapSO3Toso3(local_rot_dcm)));
				break;
			}
			case Scale: {				
				auto scale = selected_obj->getScale();
				scale += dir;
				selected_obj->setScale(scale);
			}
			}
		}

	}
	else if (picking_msg_ && picking_msg_->drawID == static_cast<float>(PassType::AuxiliaryPass)) {
		if (picking_msg_->primitiveID <= m_cur_axis->GetStridedSize())
		{
			m_axis_selected = AxisSelected::xAxis;
		}
		else if (picking_msg_->primitiveID <= 2 * m_cur_axis->GetStridedSize())
		{
			m_axis_selected = AxisSelected::yAxis;
		}
		else
		{
			m_axis_selected = AxisSelected::zAxis;
		}
	}
	else{
			m_axis_selected = AxisSelected::None;
	}
	
	if (Model* selected_obj = ModelManager::getInstance().GetModelByHandle(selected_id);
		selected_obj)
	{
		Camera* camera = ModelManager::getInstance().GetCameraByHandle(1);
		mat4 view = camera->GetViewMatrix();
		mat4 model_mat = selected_obj->getModelMatrix();
		vec4 glb_pos(model_mat[3][0], model_mat[3][1], model_mat[3][2], 1.0f);
		vec3 pos = vec3(glm::inverse(view) * glb_pos);
		scale = vec3(glm::l2Norm(pos) * 0.05f);
	
		m_cur_axis->SetAxisSelected(m_axis_selected);
		m_cur_axis->setModelMatrix(
			(m_axis_mode != AxisMode::Scale? selected_obj->getTranslationModelMatrix() : selected_obj->getModelMatrixWithoutScale())
				* glm::scale(glm::mat4(1.0f), scale));
		m_cur_axis->tick();
	}

}

void GComponent::UIState::SetGL(const shared_ptr<MyGL>& gl)
{
	picking_controller.SetGL(gl);
	picking_controller.Init(m_width, m_height);	
}

GComponent::PickingPixelInfo GComponent::UIState::GetPickingPixelInfo()
{
	return picking_controller.GetPickingPixelInfo(m_mouse_pos_x, m_height - m_mouse_pos_y - 1);
}

GComponent::Model* GComponent::UIState::GetSelectedObject() const
{
	return ModelManager::getInstance().GetModelByHandle(selected_id);
}

/*__________________________________PRIVATE METHODS_____________________________________________*/
void GComponent::UIState::ProcessDelete()
{
	static bool has_response_delete = false;
	if (!has_response_delete)
	{
		if (m_key_state & static_cast<size_t>(KeyButtonState::KeyDelete) && selected_id != NoneSelected)
		{
			has_response_delete = true;
			emit DeleteRequest(ModelManager::getInstance().GetNameByID(selected_id));
			ModelManager::getInstance().DeregisteredModel(selected_id);
			selected_id = NoneSelected;
		}
	}
	else
	{
		if (~m_key_state & static_cast<size_t>(KeyButtonState::KeyDelete))
		{
			has_response_delete = false;
		}
	}
}

void GComponent::UIState::Init(int segments, float radius)
{
	if (!is_init) {
		m_translation_axis->Init(segments, radius);
		m_scale_axis->Init(segments, radius);
		m_rotation_axis->Init(5 * segments, 30.0f * radius);
	}
	is_init = true;
}

/*____________________________________UI PROCESS_________________________________________________*/
void GComponent::UIState::OnCursorMove(int mouse_pos_x, int mouse_pos_y)
{
	m_mouse_pos_x = mouse_pos_x;
	m_mouse_pos_y = mouse_pos_y;
}

void GComponent::UIState::OnMousePress(unsigned button_flag)
{
	button_state = button_flag;
	if (button_state & MouseButton::LeftButton) 
	{
		if (picking_msg_) 
		{
			if (picking_msg_->drawID == static_cast<float>(PassType::DirLightPass)) 
			{
				selected_id = picking_msg_->modelID;			
			}
			else if (picking_msg_->drawID == static_cast<float>(PassType::AuxiliaryPass)) 
			{
				is_draged	= true;
			}
			else
			{
				selected_id = 0;
			}
			emit SelectRequest(ModelManager::getInstance().GetNameByID(selected_id));
		}			
	}
	if (button_state & MouseButton::RightButton) 
	{

	}
	if (button_state & MouseButton::MiddleButton)
	{

	}
}

void GComponent::UIState::OnMouseRelease(unsigned button_flag)
{
	button_state &= (~button_flag);
	if (!(button_state & MouseButton::LeftButton))
	{
		is_draged = false;
	}
}

void GComponent::UIState::OnMouseEnter(int mouse_pos_x, int mouse_pos_y)
{
	is_enter_area = true;
	m_mouse_pos_x = m_last_mouse_pos_x = mouse_pos_x;
	m_mouse_pos_y = m_last_mouse_pos_y = mouse_pos_y;
}

void GComponent::UIState::OnMouseLeave()
{
	is_enter_area = false;
}

void GComponent::UIState::OnKeyPress(size_t key_state)
{
	m_key_state |= key_state;
}

void GComponent::UIState::OnKeyRelease(size_t key_state)
{
	m_key_state &= ~key_state;
}

void GComponent::UIState::OnResize(int w, int h)
{
	m_width = w, m_height = h;
	m_aspect = static_cast<float>(w) / h;
	picking_controller.Init(m_width, m_height);
}

/*_____________________________________SLOT FUNCTIONS________________________________________________*/
void GComponent::UIState::ResponseSelectRequest(const string& select_obj_name) 
{
	selected_id_buffer = ModelManager::getInstance().GetIDByName(select_obj_name);
}

void GComponent::UIState::ResponseAxisModeChange(AxisMode mode)
{
	m_axis_mode = mode;
}
