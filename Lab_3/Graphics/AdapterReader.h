#pragma once

#include <vector>
#include <wrl/client.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DXGI.lib")

class AdapterData
{
public:
	AdapterData(IDXGIAdapter* adapter_pointer);
	IDXGIAdapter* adapter_pointer = nullptr;
	DXGI_ADAPTER_DESC description;

};

class AdapterReader
{
public:
	static std::vector<AdapterData> GetAdapters();
private:
	static std::vector<AdapterData> m_adapters;
};
