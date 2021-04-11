#pragma once
#include <queue>

#include "MouseEvent.h"

class MouseClass
{
public:
	void OnLeftPressed(int x, int y);
	void OnLeftReleased(int x, int y);
	void OnRightPressed(int x, int y);
	void OnRightReleased(int x, int y);
	void OnMiddlePressed(int x, int y);
	void OnMiddleReleased(int x, int y);
	void OnWheelUp(int x, int y);
	void OnWheelDown(int x, int y);
	void OnMouseMove(int x, int y);
	void OnMouseMoveRaw(int x, int y);

	bool IsLeftDown() const;
	bool IsMiddleDown() const;
	bool IsRightDown() const;

	int GetPosX() const;
	int GetPosY() const;
	MousePoint GetPos() const;

	bool IsEventBufferEmpty() const;
	MouseEvent ReadEvent();

private:
	std::queue<MouseEvent> m_event_buffer;
	bool m_is_left_down = false;
	bool m_is_right_down = false;
	bool m_is_middle_down = false;
	int m_x = 0;
	int m_y = 0;
};
