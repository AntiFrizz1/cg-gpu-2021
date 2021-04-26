#include "StringConverter.h"

namespace utils
{

	std::wstring StringConverter::StringToWide(std::string str)
	{
		return std::wstring(str.begin(), str.end());
	}
}
