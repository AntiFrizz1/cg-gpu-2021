#include "AdapterReader.h"
#include "../WinErrorLoger.h"

std::vector<AdapterData> AdapterReader::m_adapters;

std::vector<AdapterData> AdapterReader::GetAdapters()
{
	if (!m_adapters.empty())
		return m_adapters;

	Microsoft::WRL::ComPtr<IDXGIFactory> factory_ptr;

	HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)factory_ptr.GetAddressOf());
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create DXGIFactory for enumerating adapters.");
		exit(-1);
	}

	IDXGIAdapter* adapter_pointer;
	UINT index = 0;
	while (SUCCEEDED(factory_ptr->EnumAdapters(index, &adapter_pointer)))
	{
		m_adapters.push_back(AdapterData(adapter_pointer));
		index += 1;
	}

	return m_adapters;
}

AdapterData::AdapterData(IDXGIAdapter * adapter_pointer)
{
	this->adapter_pointer = adapter_pointer;
	HRESULT hr = adapter_pointer->GetDesc(&description);
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to get description for IDXGIAdapter");
	}
}
