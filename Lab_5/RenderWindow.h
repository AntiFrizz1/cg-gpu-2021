#pragma once
#include "WinErrorLoger.h"

class WindowContainer;

class RenderWindow
{
public:
	bool Initialize(WindowContainer* window_container_ptr,
					HINSTANCE hInstance,
					std::string window_title, 
					std::string window_class_name, 
					int width, 
					int height);
	bool ProcessMessages();
	HWND GetHwnd() const;
	~RenderWindow();

private:
	bool register_window_class();
	
	HWND m_handle = NULL;
	HINSTANCE m_hIsnstance = NULL;
	std::string m_window_title = "";
	std::wstring m_window_title_wide = L"";
	std::string m_window_class_name = "";
	std::wstring m_window_class_name_wide = L"";
	int m_width = 0;
	int m_height = 0;
};
