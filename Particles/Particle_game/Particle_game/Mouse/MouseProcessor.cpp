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
			float const speed = 1.0f / 4;
			MouseEvent mouse_event = m_mouse_ptr->ReadEvent();
			if (mouse_event.GetType() == MouseEvent::EventType::RPress)
			{
				m_mouse_pressed = true;
			}
			if (mouse_event.GetType() == MouseEvent::EventType::RRelease)
			{
				m_mouse_pressed = false;
			}
			
			CameraOld* camera = m_graphics_ptr->GetCamera();
			if (mouse_event.GetType() == MouseEvent::EventType::WheelUp)
			{
				camera->MoveUp(speed);
			}
			if (mouse_event.GetType() == MouseEvent::EventType::WheelDown)
			{
				camera->MoveUp(-speed);
			}
			if (m_mouse_pressed && mouse_event.GetType() == MouseEvent::EventType::RAW_MODE)
			{
				int dx = mouse_event.GetPosX();
				int dy = mouse_event.GetPosY();

				float lon = (float)dx / 100.f;
				float lat = (float)dy / 100.f;

				camera->AddLat(lat);
				camera->AddLon(lon);
			}
		}
	}
}