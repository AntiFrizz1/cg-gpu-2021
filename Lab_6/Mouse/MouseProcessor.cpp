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
			
			auto& camera_position = m_graphics_ptr->RefWorldCameraPosition();
			float& lon = camera_position.lon;
			float& lat = camera_position.lat;
			float& pos_x = camera_position.pos_x;
			float& pos_y = camera_position.pos_y;
			float& pos_z = camera_position.pos_z;
			DirectX::XMMATRIX& view = m_graphics_ptr->RefView();

			if (mouse_event.GetType() == MouseEvent::EventType::WheelUp)
			{
				pos_z += speed;
				view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis({ 1,0,0 }, lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, lon) * DirectX::XMMatrixTranslation(pos_x, pos_y, pos_z));
			}
			if (mouse_event.GetType() == MouseEvent::EventType::WheelDown)
			{
				pos_z -= speed;
				view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis({ 1,0,0 }, lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, lon) * DirectX::XMMatrixTranslation(pos_x, pos_y, pos_z));
			}
			if (m_mouse_pressed && mouse_event.GetType() == MouseEvent::EventType::RAW_MODE)
			{
				int dx = mouse_event.GetPosX();
				int dy = mouse_event.GetPosY();

				lon += (float)dx / 100.f;
				lat += (float)dy / 100.f;

				if (lat <= -(float)DirectX::XM_PI / 2)
				{
					lat = -(float)DirectX::XM_PI / 2;
				}
				if (lat >= (float)DirectX::XM_PI / 2)
				{
					lat = (float)DirectX::XM_PI / 2;
				}
				view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis({ 1,0,0 }, lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, lon) * DirectX::XMMatrixTranslation(pos_x, pos_y, pos_z));
			}
		}
	}
}