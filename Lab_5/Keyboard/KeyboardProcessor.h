#pragma once
#include "KeyboardClass.h"
#include "KeyboardEvent.h"
#include "../Graphics/Graphics.h"

class KeyboardProcessor
{
public:
	KeyboardProcessor() = default;
	bool Initialize(KeyboardClass* keyboard_ptr, Graphics* graphics_ptr);
	void Process();
private:
	KeyboardClass* m_keyboard_ptr{ nullptr };
	Graphics*      m_graphics_ptr{ nullptr };
};