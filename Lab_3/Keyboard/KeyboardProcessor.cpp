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
			
			auto& camera_position = m_graphics_ptr->RefWorldCameraPosition();
			float& lon = camera_position.lon;
			float& lat = camera_position.lat;
			float& pos_x = camera_position.pos_x;
			float& pos_y = camera_position.pos_y;
			float& pos_z = camera_position.pos_z;
			DirectX::XMMATRIX& view = m_graphics_ptr->RefView();
			
			if (key_event.IsPress() && key_event.GetKeyCode() == 'W')
			{
				pos_y += speed;
				view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis({ 1,0,0 }, lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, lon) * DirectX::XMMatrixTranslation(pos_x, pos_y, pos_z));
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == 'S')
			{
				pos_y -= speed;
				view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis({ 1,0,0 }, lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, lon) * DirectX::XMMatrixTranslation(pos_x, pos_y, pos_z));
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == 'A')
			{
				pos_x -= speed;
				view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis({ 1,0,0 }, lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, lon) * DirectX::XMMatrixTranslation(pos_x, pos_y, pos_z));
			}
			if (key_event.IsPress() && key_event.GetKeyCode() == 'D')
			{
				pos_x += speed;
				view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis({ 1,0,0 }, lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, lon) * DirectX::XMMatrixTranslation(pos_x, pos_y, pos_z));
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
