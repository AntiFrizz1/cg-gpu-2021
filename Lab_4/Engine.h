#pragma once
#include "WindowContainer.h"
#include "Keyboard/KeyboardProcessor.h"
#include "Mouse/MouseProcessor.h"

class Engine : WindowContainer
{
public:
	bool Initialize(HINSTANCE hInstance,
		std::string window_title,
		std::string window_class_name,
		int width,
		int height,
		bool debug_mode = false);
	bool ProcessMessages();
	void Update();
	void RenderFrame();
private:
	virtual bool try_resize_window(size_t width, size_t height) final;

	KeyboardProcessor m_keyboard_processor;
	MouseProcessor    m_mouse_processor;
};
