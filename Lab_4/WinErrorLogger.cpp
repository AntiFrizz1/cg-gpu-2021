#include <comdef.h>

#include "WinErrorLoger.h"

namespace utils
{
	void WinErrorLogger::Log(std::string message)
	{
		std::string error_message = "Error: " + message;
		MessageBoxA(NULL, error_message.c_str(), "Error", MB_ICONERROR); // MessageBoxA(parent window, message, title, message box type)
	}

	void WinErrorLogger::Log(HRESULT hr, std::string message)
	{
		Log(hr, StringConverter::StringToWide(message));
	}

	void WinErrorLogger::Log(HRESULT hr, std::wstring message)
	{
		_com_error error(hr);
		std::wstring error_message = L"Error: " + message + L"\n" + error.ErrorMessage();
		MessageBoxW(NULL, error_message.c_str(), L"Error", MB_ICONERROR); // MessageBoxW(parent window, message, title, message box type)
	}
}
