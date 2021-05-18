#include "RenderWindow.h"
#include "WindowContainer.h"

bool RenderWindow::Initialize(WindowContainer* window_container_ptr, HINSTANCE hInstance, std::string window_title, std::string window_class_name, int width, int height)
{
	m_hIsnstance = hInstance;
	m_width = width;
	m_height = height;
	m_window_title = window_title;
	m_window_title_wide = utils::StringConverter::StringToWide(window_title);
	m_window_class_name = window_class_name;
	m_window_class_name_wide = utils::StringConverter::StringToWide(window_class_name);

	if (!register_window_class())
	{
		utils::WinErrorLogger::Log("Failed to register window class.");
		return false;
	}

	RECT wr; // Window Rectangle
	wr.left = 50;
	wr.top = 50;
	wr.right = wr.left +  width;
	wr.bottom = wr.top +  height;
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

	m_handle = CreateWindowEx(0,
		m_window_class_name_wide.c_str(),
		m_window_title_wide.c_str(),
		WS_OVERLAPPEDWINDOW,
		wr.left,
		wr.top,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL,
		NULL,
		m_hIsnstance,
		window_container_ptr);

	if (m_handle == NULL)
	{
		utils::WinErrorLogger::Log(GetLastError(), "CreateWindowEX failed for window: " + m_window_title);
		return false;
	}

	ShowWindow(m_handle, SW_SHOW);

	return true;
}

bool RenderWindow::ProcessMessages()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	while (PeekMessage(&msg, m_handle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_NULL)
	{
		if (!IsWindow(m_handle))
		{
			m_handle = NULL;
			UnregisterClass(m_window_class_name_wide.c_str(), m_hIsnstance);
			return false;
		}
	}

	return true;
}

HWND RenderWindow::GetHwnd() const
{
	return m_handle;
}

RenderWindow::~RenderWindow()
{
	if (m_handle != NULL)
	{
		UnregisterClass(m_window_class_name_wide.c_str(), m_hIsnstance);
		DestroyWindow(m_handle);
	}
}

LRESULT CALLBACK HandleMsgRedirect(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
		return 0;
	}
	default:
		// retrieve window container ptr
		WindowContainer* window_container_ptr = reinterpret_cast<WindowContainer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		// forward message to window container handler
		return window_container_ptr->WindowProc(hwnd, uMsg, wParam, lParam);
	}
}

LRESULT CALLBACK HandleMessageSetup(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCREATE:
	{
		CREATESTRUCTW const* pointer_create = reinterpret_cast<CREATESTRUCTW*>(lParam);
		WindowContainer* window_container_ptr = reinterpret_cast<WindowContainer*>(pointer_create->lpCreateParams);
		if (!window_container_ptr)
		{
			utils::WinErrorLogger::Log("Critical Error: Pointer of WindowContainer is null during WM_NCCREATE");
			exit(-1);
		}
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window_container_ptr));
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HandleMsgRedirect));
		return window_container_ptr->WindowProc(hwnd, uMsg, wParam, lParam);
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

bool RenderWindow::register_window_class()
{
	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = HandleMessageSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hIsnstance;
	wc.hIcon = NULL;
	wc.hIconSm = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_window_class_name_wide.c_str();
	wc.cbSize = sizeof(WNDCLASSEX);
	return RegisterClassEx(&wc);
}
