#include "Engine.h"
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>
#include "Global.h"

bool Engine::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class_name, int width, int height, bool debug_mode)
{
	m_keyboard.EnableAutoRepeatKeys();
	return m_render_window.Initialize(this, hInstance, window_title, window_class_name, width, height) 
		&& m_graphics.Initialize(m_render_window.GetHwnd(), width, height) 
		&& m_keyboard_processor.Initialize(&m_keyboard, &m_graphics)
		&& m_mouse_processor.Initialize(&m_mouse, &m_graphics);
}

bool Engine::ProcessMessages()
{
	return m_render_window.ProcessMessages();
}

void Engine::Update()
{
	m_keyboard_processor.Process();
	m_mouse_processor.Process();

}

void Engine::RenderFrame()
{
	m_graphics.RenderFrame();
}

bool Engine::try_resize_window(size_t width, size_t height)
{
	Global::GetAnnotation().BeginEvent(L"Start resizing");
	bool res = m_graphics.OnResizeWindow(width, height);
	Global::GetAnnotation().EndEvent();
	return res;
}