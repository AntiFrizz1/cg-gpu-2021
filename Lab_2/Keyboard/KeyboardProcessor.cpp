#include "KeyboardProcessor.h"
#include "../Global.h"

bool KeyboardProcessor::Initialize(KeyboardClass* keyboard_ptr, Graphics* graphics_ptr)
{
	m_keyboard_ptr = keyboard_ptr;
	m_graphics_ptr = graphics_ptr;
	return m_keyboard_ptr && m_graphics_ptr;
}

void KeyboardProcessor::Process()
{
	if (m_keyboard_ptr && m_graphics_ptr)
	{
		while (!m_keyboard_ptr->IsKeyBufferEmpty())
		{
			KeyboardEvent key_event = m_keyboard_ptr->ReadKey();
			float const speed = 0.5f;
			std::wstring annotation_message = L"Start keyboard event " + key_event.GetKeyCode();
			annotation_message += L"'";
			Global::GetAnnotation().BeginEvent(annotation_message.c_str());
			if (key_event.IsPress() && key_event.GetKeyCode() == 'W')
			{
				DirectX::XMMATRIX& view = m_graphics_ptr->RefView();
				view._41 -= view._12 * speed;
				view._42 -= view._22 * speed;
				view._43 -= view._32 * speed;
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == 'S')
			{
				DirectX::XMMATRIX& view = m_graphics_ptr->RefView();
				view._41 += view._12 * speed;
				view._42 += view._22 * speed;
				view._43 += view._32 * speed;
			}

			if (key_event.IsPress() && key_event.GetKeyCode() == 'A')
			{
				DirectX::XMMATRIX& view = m_graphics_ptr->RefView();
				view._41 += view._11 * speed;
				view._42 += view._21 * speed;
				view._43 += view._31 * speed;
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == 'D')
			{
				DirectX::XMMATRIX& view = m_graphics_ptr->RefView();
				view._41 -= view._11 * speed;
				view._42 -= view._21 * speed;
				view._43 -= view._31 * speed;
			}

			if (key_event.IsPress() && key_event.GetKeyCode() == '1')
			{
				m_graphics_ptr->ChangeLightsIntencity(0);
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == '2')
			{
				m_graphics_ptr->ChangeLightsIntencity(1);
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == '3')
			{
				m_graphics_ptr->ChangeLightsIntencity(2);
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == '4')
			{
				m_graphics_ptr->ChangeToneMaping();
			}
			Global::GetAnnotation().EndEvent();
		}
	}
}
