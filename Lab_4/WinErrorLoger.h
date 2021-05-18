#pragma once
#include <Windows.h>

#include "StringConverter.h"

namespace utils
{
	class WinErrorLogger
	{
	public:
		void static Log(std::string message);
		void static Log(HRESULT hr, std::string message);
		void static Log(HRESULT hr, std::wstring message);
	};
}
