#pragma once

class KeyboardEvent
{
public:
	enum class EventType
	{
		Press,
		Release,
		Invalid
	};

	KeyboardEvent();
	KeyboardEvent(EventType type, unsigned char key);

	bool IsPress() const;
	bool IsRelease() const;
	bool IsValid() const;
	
	unsigned char GetKeyCode() const;

private:
	EventType m_type;
	unsigned char m_key;
};
