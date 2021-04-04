#include "Engine.h"
#include "Global.h"
bool extract_debug_arg(LPWSTR lpCmdLine)
{
	int argCount;
	LPWSTR* argList;
	bool debug_mode = false;
	argList = CommandLineToArgvW(lpCmdLine, &argCount);
	if (argCount > 0)
	{
		std::wstring arg = argList[0];
		if (arg == L"DEBUG")
		{
			return true;
		}
	}
	return false;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Global::GetInstance().debug_mode = extract_debug_arg(lpCmdLine);

	Engine engine;
	if (engine.Initialize(hInstance, "WindowClass", "lab_2", 800, 600, false))
	{
		while (engine.ProcessMessages() == true)
		{
			engine.Update();
			engine.RenderFrame();
		}
	}
	return 0;
}