#pragma once
#include "RenderWindow.h"
#include "Keyboard/KeyboardClass.h"
#include "Mouse/MouseClass.h"
#include "WinErrorLoger.h"
#include "Graphics/Graphics.h"

class WindowContainer
{
public:
	WindowContainer();
	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	virtual bool try_resize_window(size_t width, size_t height) { return true; };

	RenderWindow m_render_window;
	KeyboardClass m_keyboard;
	MouseClass m_mouse;
	Graphics m_graphics;
private:
};
