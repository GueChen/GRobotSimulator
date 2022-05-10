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


GComponent::UIState::UIState(unsigned w, unsigned h):
	m_width(w), m_height(h)
{
	m_translation_axis	= new QtGLTranslationAxis;
	m_scale_axis		= new QtGLScaleAxis;
	m_rotation_axis		= new QtGLRotationAxis;

	ModelManager::getInstance().RegisteredAuxiModel(m_translation_axis->getName(), m_translation_axis);
	ModelManager::getInstance().RegisteredAuxiModel(m_rotation_axis->getName(),	   m_scale_axis);
	ModelManager::getInstance().RegisteredAuxiModel(m_scale_axis->getName(),       m_rotation_axis);
}

void GComponent::UIState::Init(int segments, float radius)
{
	if (!have_init) {
		m_translation_axis->Init(segments, radius);
		m_scale_axis->Init(segments, radius);
		m_rotation_axis->Init(segments, 50.0f * radius);
	}
	have_init = true;
}

void GComponent::UIState::tick()
{

	static bool has_response_delete = false;
	if (!has_response_delete) 
	{
		if (m_key_state & static_cast<size_t>(KeyButtonState::KeyDelete) && selected_id != NoneSelected) 
		{
			has_response_delete = true;		
			Model* obj = ModelManager::getInstance().GetModelByHandle(selected_id);
			Model* parent_obj = obj->getParent();
			while (parent_obj && parent_obj->getMesh().empty()) 
			{
				obj = parent_obj;
				parent_obj = obj->getParent();
			}
			emit DeleteRequest(obj->getName());	
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

	if (is_enter_area) 
	{
		picking_msg_ = GetPickingPixelInfo();
		RenderManager::getInstance().SetPickingController(picking_controller);
	}
	else 
	{
		picking_msg_ = std::nullopt;
	}

	if (selected_id != NoneSelected) 
	{
		switch (m_axis_mode) 
		{
			using enum AxisMode;
		case Translation: 
		{
			m_cur_axis = m_translation_axis;
			break;
		}
		case Rotation: 
		{
			m_cur_axis = m_rotation_axis;
			break;
		}
		case Scale: 
		{
			m_cur_axis = m_scale_axis;
			break;
		}
		default:
			throw std::exception("The AxisMode Not Define!");
		}

		Model* selected_obj = ModelManager::getInstance().GetModelByHandle(selected_id);
		m_cur_axis->SetAxisSelected(m_axis_selected);
		m_cur_axis->setModelMatrix(selected_obj->getModelMatrix() * glm::scale(glm::mat4(1.0f), scale));
		m_cur_axis->tick();
	}

	if(!is_draged)
	if (picking_msg_ && picking_msg_->drawID == static_cast<float>(PassType::AuxiliaryPass)) {
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

#ifdef _DEBUGGING
	if (picking_msg_)
	{
		auto & msg = picking_msg_.value();
		std::cout << std::format("cur FBO       - {: >5}\n", QOpenGLContext::currentContext()->defaultFramebufferObject());
		std::cout << std::format("cur mouse pos - [{: >5}, {: >5}]\n", m_mouse_pos_x, (int)m_height - m_mouse_pos_y - 1);
		std::cout << std::format("cur pixel info:\n"
			"Draw Pass ID  - {: >5}\n"
			"Model ID      - {: >5}\n"
			"Primitive ID  - {: >5}\n", msg.drawID, msg.modelID, msg.primitiveID);
	}
#endif // _DEBUG	
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

void GComponent::UIState::OnMouseEnter()
{
	is_enter_area = true;
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
	picking_controller.Init(m_width, m_height);
}

