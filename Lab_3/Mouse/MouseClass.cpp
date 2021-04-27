#include "MouseClass.h"

void MouseClass::OnLeftPressed(int x, int y)
{
	m_is_left_down = true;
	m_event_buffer.emplace(MouseEvent::EventType::LPress, x, y);
}

void MouseClass::OnLeftReleased(int x, int y)
{
	m_is_left_down = false;
	m_event_buffer.emplace(MouseEvent::EventType::LRelease, x, y);
}

void MouseClass::OnRightPressed(int x, int y)
{
	m_is_right_down = true;
	m_event_buffer.emplace(MouseEvent::EventType::RPress, x, y);
}

void MouseClass::OnRightReleased(int x, int y)
{
	m_is_right_down = false;
	m_event_buffer.emplace(MouseEvent::EventType::RRelease, x, y);
}

void MouseClass::OnMiddlePressed(int x, int y)
{
	m_is_middle_down = true;
	m_event_buffer.emplace(MouseEvent::EventType::MPress, x, y);
}

void MouseClass::OnMiddleReleased(int x, int y)
{
	m_is_middle_down = false;
	m_event_buffer.emplace(MouseEvent::EventType::MRelease, x, y);
}

void MouseClass::OnWheelUp(int x, int y)
{
	m_event_buffer.emplace(MouseEvent::EventType::WheelUp, x, y);
}

void MouseClass::OnWheelDown(int x, int y)
{
	m_event_buffer.emplace(MouseEvent::EventType::WheelDown, x, y);
}

void MouseClass::OnMouseMove(int x, int y)
{
	m_x = x;
	m_y = y;
	m_event_buffer.emplace(MouseEvent::EventType::Move, x, y);
}

void MouseClass::OnMouseMoveRaw(int x, int y)
{
	m_event_buffer.emplace(MouseEvent::EventType::RAW_MODE, x, y);
}

bool MouseClass::IsLeftDown() const
{
	return m_is_left_down;
}

bool MouseClass::IsMiddleDown() const
{
	return m_is_middle_down;
}

bool MouseClass::IsRightDown() const
{
	return m_is_right_down;
}

int MouseClass::GetPosX() const
{
	return m_x;
}

int MouseClass::GetPosY() const
{
	return m_y;
}

MousePoint MouseClass::GetPos() const
{
	return {m_x, m_y};
}

bool MouseClass::IsEventBufferEmpty() const
{
	return m_event_buffer.empty();
}

MouseEvent MouseClass::ReadEvent()
{
	if (m_event_buffer.empty())
	{
		return MouseEvent();
	}
	else
	{
		MouseEvent e = m_event_buffer.front();
		m_event_buffer.pop();
		return e;
	}
}
