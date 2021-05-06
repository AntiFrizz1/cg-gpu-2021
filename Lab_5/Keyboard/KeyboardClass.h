#pragma once

#include "KeyboardEvent.h"
#include <queue>

class KeyboardClass
{
public:
	static size_t const MAX_KEYS_COUNT = 256;

	KeyboardClass();

	bool IsKeyPressed(unsigned char keycode);
	bool IsKeyBufferEmpty();
	bool IsCharBufferEmpty();

	KeyboardEvent ReadKey();
	unsigned char ReadChar();

	void OnKeyPressed(unsigned char key);
	void OnKeyReleased(unsigned char key);
	void OnChar(unsigned char key);

	void EnableAutoRepeatKeys();
	void DisableAutoRepeatKeys();
	void EnableAutoRepeatChars();
	void DisableAutoRepeatChars();
	bool IsKeysAutoRepeat();
	bool IsCharsAutoRepeat();

private:
	bool m_auto_repeat_keys = false;
	bool m_auto_repeat_chars = false;
	bool m_key_states[MAX_KEYS_COUNT];
	std::queue<KeyboardEvent> m_key_buffer;
	std::queue<unsigned char> m_char_buffer;
};
