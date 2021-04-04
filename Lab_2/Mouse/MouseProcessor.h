#pragma once
#include "MouseClass.h"
#include "MouseEvent.h"
#include "../Graphics/Graphics.h"

class MouseProcessor
{
public:
	MouseProcessor() = default;
	bool Initialize(MouseClass* mouse_ptr, Graphics* graphics_ptr);
	void Process();
private:
	MouseClass* m_mouse_ptr{ nullptr };
	Graphics*   m_graphics_ptr{ nullptr };
	bool        m_mouse_pressed{ false };
};