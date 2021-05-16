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
			float const speed = 0.25f;
			std::wstring annotation_message = L"Start keyboard event " + key_event.GetKeyCode();
			annotation_message += L"'";
			Global::GetAnnotation().BeginEvent(annotation_message.c_str());
			CameraOld* camera = m_graphics_ptr->GetCamera();
			if (key_event.IsPress() && key_event.GetKeyCode() == 'W')
			{
				camera->MoveForvard(speed);
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == 'S')
			{
				camera->MoveForvard(-speed);
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == 'A')
			{
				camera->MoveRight(-speed);
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == 'D')
			{
				camera->MoveRight(speed);
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
				m_graphics_ptr->SwitchToneMaping();
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == '5')
			{
				m_graphics_ptr->SetPbrShaderType(PbrShaderType::BRDF);
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == '6')
			{
				m_graphics_ptr->SetPbrShaderType(PbrShaderType::NDF);
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == '7')
			{
				m_graphics_ptr->SetPbrShaderType(PbrShaderType::GEOMETRY);
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == '8')
			{
				m_graphics_ptr->SetPbrShaderType(PbrShaderType::FRESNEL);
				m_graphics_ptr->SetToneMaping(false);
			}
			Global::GetAnnotation().EndEvent();
		}
	}
}
