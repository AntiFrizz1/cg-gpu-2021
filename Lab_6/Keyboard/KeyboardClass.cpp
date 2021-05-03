#include "KeyboardClass.h"

KeyboardClass::KeyboardClass()
{
	for (int i = 0; i < MAX_KEYS_COUNT; ++i)
	{
		m_key_states[i] = false;
	}
}

bool KeyboardClass::IsKeyPressed(unsigned char keycode)
{
	return m_key_states[keycode];
}

bool KeyboardClass::IsKeyBufferEmpty()
{
	return m_key_buffer.empty();
}

bool KeyboardClass::IsCharBufferEmpty()
{
	return m_char_buffer.empty();
}

KeyboardEvent KeyboardClass::ReadKey()
{
	if (m_key_buffer.empty())
	{
		return KeyboardEvent();
	}
	else
	{
		KeyboardEvent event = m_key_buffer.front();
		m_key_buffer.pop();
		return event;
	}
}

unsigned char KeyboardClass::ReadChar()
{
	if (m_char_buffer.empty())
	{
		return 0u;
	}
	else
	{
		unsigned char c = m_char_buffer.front();
		m_char_buffer.pop();
		return c;
	}
}

void KeyboardClass::OnKeyPressed(unsigned char key)
{
	m_key_states[key] = true;
	m_key_buffer.emplace(KeyboardEvent::EventType::Press, key);
}

void KeyboardClass::OnKeyReleased(unsigned char key)
{
	m_key_states[key] = false;
	m_key_buffer.emplace(KeyboardEvent::EventType::Release, key);
}

void KeyboardClass::OnChar(unsigned char key)
{
	m_char_buffer.push(key);
}


void KeyboardClass::EnableAutoRepeatKeys()
{
	m_auto_repeat_keys = true;
}

void KeyboardClass::DisableAutoRepeatKeys()
{
	m_auto_repeat_keys = false;
}

void KeyboardClass::EnableAutoRepeatChars()
{
	m_auto_repeat_chars = true;
}

void KeyboardClass::DisableAutoRepeatChars()
{
	m_auto_repeat_chars = false;
}

bool KeyboardClass::IsKeysAutoRepeat()
{
	return m_auto_repeat_keys;
}

bool KeyboardClass::IsCharsAutoRepeat()
{
	return m_auto_repeat_chars;
}