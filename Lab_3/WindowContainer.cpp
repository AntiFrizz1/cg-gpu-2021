#include <memory>

#include "WindowContainer.h"

WindowContainer::WindowContainer()
{
	static bool raw_input_initialized = false;
	if (!raw_input_initialized)
	{
		RAWINPUTDEVICE rid;

		rid.usUsagePage = 0x01; // this means mouse
		rid.usUsage = 0x02; // this means mouse
		rid.dwFlags = 0;
		rid.hwndTarget = NULL;

		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
		{
			utils::WinErrorLogger::Log(GetLastError(), "Failed to register raw input device.");
			exit(-1);
		}
	}
	raw_input_initialized = true;
}

LRESULT WindowContainer::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	// Keyboard messages
	case WM_CHAR:
	{
		uint8_t ch = static_cast<uint8_t>(wParam);
		if (m_keyboard.IsCharsAutoRepeat())
		{
			m_keyboard.OnChar(ch);
		}
		else
		{
			bool const was_pressed = lParam & 0x40000000;
			if (!was_pressed)
			{
				m_keyboard.OnChar(ch);
			}
		}
		return 0;
	}
	case WM_KEYDOWN:
	{
		uint8_t keycode = static_cast<uint8_t>(wParam);
		if (m_keyboard.IsKeysAutoRepeat())
		{
			m_keyboard.OnKeyPressed(keycode);
		}
		else
		{
			bool const was_pressed = lParam & 0x40000000;
			if (!was_pressed)
			{
				m_keyboard.OnKeyPressed(keycode);
			}
		}
		return 0;
	}
	case WM_KEYUP:
	{
		uint8_t keycode = static_cast<uint8_t>(wParam);
		m_keyboard.OnKeyReleased(keycode);
		return 0;
	}
	// Mouse messages
	case WM_MOUSEMOVE:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		m_mouse.OnMouseMove(x, y);
		return 0;
	}
	case WM_LBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		m_mouse.OnLeftPressed(x, y);
		return 0;
	}
	case WM_LBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		m_mouse.OnLeftReleased(x, y);
		return 0;
	}
	case WM_RBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		m_mouse.OnRightPressed(x, y);
		return 0;
	}
	case WM_RBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		m_mouse.OnRightReleased(x, y);
		return 0;
	}
	case WM_MBUTTONDOWN:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		m_mouse.OnMiddlePressed(x, y);
		return 0;
	}
	case WM_MBUTTONUP:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		m_mouse.OnMiddleReleased(x, y);
		return 0;
	}
	case WM_MOUSEWHEEL:
	{
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
		{
			m_mouse.OnWheelUp(x, y);
		}
		else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
		{
			m_mouse.OnWheelDown(x, y);
		}
		return 0;
	}
	case WM_INPUT:
	{
		UINT data_size;
		HRAWINPUT raw_input = reinterpret_cast<HRAWINPUT>(lParam);
		GetRawInputData(raw_input, RID_INPUT, NULL, &data_size, sizeof(RAWINPUTHEADER));

		if (data_size > 0)
		{
			std::unique_ptr<BYTE[]> rawdata = std::make_unique<BYTE[]>(data_size);
			if (GetRawInputData(raw_input, RID_INPUT, rawdata.get(), &data_size, sizeof(RAWINPUTHEADER)) == data_size)
			{
				RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(rawdata.get());
				if (raw->header.dwType == RIM_TYPEMOUSE)
				{
					m_mouse.OnMouseMoveRaw(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
				}
			}
		}


		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	// Resize Window
	case WM_SIZE:
	{
		UINT width = LOWORD(lParam);
		UINT height = HIWORD(lParam);
		if (!try_resize_window(width, height))
		{
			PostQuitMessage(1);
		}
		return 0;
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}
