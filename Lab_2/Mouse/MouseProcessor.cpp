#include "MouseProcessor.h"

bool MouseProcessor::Initialize(MouseClass* mouse_ptr, Graphics* graphics_ptr)
{
	m_mouse_ptr = mouse_ptr;
	m_graphics_ptr = graphics_ptr;
	return m_mouse_ptr && m_graphics_ptr;
}

void MouseProcessor::Process()
{
	if (m_mouse_ptr && m_graphics_ptr)
	{
		while (!m_mouse_ptr->IsEventBufferEmpty())
		{
			float const speed = 1.0f;
			MouseEvent mouse_event = m_mouse_ptr->ReadEvent();
			if (mouse_event.GetType() == MouseEvent::EventType::LPress)
			{
				m_mouse_pressed = true;
			}
			if (mouse_event.GetType() == MouseEvent::EventType::LRelease)
			{
				m_mouse_pressed = false;
			}
			if (mouse_event.GetType() == MouseEvent::EventType::WheelUp)
			{
				DirectX::XMMATRIX& view = m_graphics_ptr->RefView();
				view._41 -= view._13 * speed;
				view._42 -= view._23 * speed;
				view._43 -= view._33 * speed;
			}
			if (mouse_event.GetType() == MouseEvent::EventType::WheelDown)
			{
				DirectX::XMMATRIX& view = m_graphics_ptr->RefView();
				view._41 += view._13 * speed;
				view._42 += view._23 * speed;
				view._43 += view._33 * speed;
			}
			if (m_mouse_pressed && mouse_event.GetType() == MouseEvent::EventType::RAW_MODE)
			{
				int dx = mouse_event.GetPosX();
				int dy = mouse_event.GetPosY();
				DirectX::XMMATRIX& view = m_graphics_ptr->RefView();
				view *= DirectX::XMMatrixRotationY(-dx / 100.f);
				view *= DirectX::XMMatrixRotationX(-dy / 100.f);
			}
		}
	}
}
