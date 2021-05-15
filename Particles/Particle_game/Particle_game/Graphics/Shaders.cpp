#include "Shaders.h"

bool VertexShader::Initialize(DeviceComPtr device_ptr, std::wstring shaderpath, D3D11_INPUT_ELEMENT_DESC* layout_desc, UINT num_elements)
{
	HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), m_shader_buffer_ptr.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring error_msg = L"Failed to load shader: " + shaderpath;
		utils::WinErrorLogger::Log(hr, error_msg);
		return false;
	}

	hr = device_ptr->CreateVertexShader(m_shader_buffer_ptr->GetBufferPointer(), m_shader_buffer_ptr->GetBufferSize(), NULL, m_shader_ptr.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring error_msg = L"Failed to create vertex shader: " + shaderpath;
		utils::WinErrorLogger::Log(hr, error_msg);
		return false;
	}
	if (layout_desc != NULL)
	{
		hr = device_ptr->CreateInputLayout(layout_desc, num_elements, m_shader_buffer_ptr->GetBufferPointer(), m_shader_buffer_ptr->GetBufferSize(), m_input_layout_ptr.GetAddressOf());
		if (FAILED(hr))
		{
			utils::WinErrorLogger::Log(hr, "Failed to create input layout");
			return false;
		}
	}

	return true;
}

bool VertexShader::Initialize(DeviceComPtr device_ptr, WCHAR* shader_path, D3D11_INPUT_ELEMENT_DESC* layout_desc, UINT num_elements, LPCSTR sz_entry_point, LPCSTR sz_shader_model, D3D_SHADER_MACRO* p_defines)
{
	HRESULT hr = S_OK;

	DWORD dw_shader_flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dw_shader_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	Microsoft::WRL::ComPtr<ID3DBlob> err;
	hr = D3DCompileFromFile(shader_path, p_defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, sz_entry_point, sz_shader_model, dw_shader_flags, 0, m_shader_buffer_ptr.GetAddressOf(), &err);
	if (FAILED(hr) && err)
		OutputDebugStringA(reinterpret_cast<const char*>(err->GetBufferPointer()));


	hr = device_ptr->CreateVertexShader(m_shader_buffer_ptr->GetBufferPointer(), m_shader_buffer_ptr->GetBufferSize(), NULL, m_shader_ptr.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring error_msg = L"Failed to create vertex shader";
		utils::WinErrorLogger::Log(hr, error_msg);
		return false;
	}
	if (layout_desc != NULL)
	{
		hr = device_ptr->CreateInputLayout(layout_desc, num_elements, m_shader_buffer_ptr->GetBufferPointer(), m_shader_buffer_ptr->GetBufferSize(), m_input_layout_ptr.GetAddressOf());
		if (FAILED(hr))
		{
			utils::WinErrorLogger::Log(hr, "Failed to create input layout");
			return false;
		}
	}

	return true;
}

ID3D11VertexShader* VertexShader::GetShaderPtr()
{
	return m_shader_ptr.Get();
}

ID3D10Blob* VertexShader::GetBufferPtr()
{
	return m_shader_buffer_ptr.Get();
}

bool PixelShader::Initialize(DeviceComPtr device_ptr, std::wstring shaderpath)
{
	HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), m_shader_buffer_ptr.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring error_msg = L"Failed to load shader: " + shaderpath;
		utils::WinErrorLogger::Log(hr, error_msg);
		return false;
	}

	hr = device_ptr->CreatePixelShader(m_shader_buffer_ptr->GetBufferPointer(), m_shader_buffer_ptr->GetBufferSize(), NULL, m_shader_ptr.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring error_msg = L"Failed to create vertex shader: " + shaderpath;
		utils::WinErrorLogger::Log(hr, error_msg);
		return false;
	}

	return true;
}

bool PixelShader::Initialize(DeviceComPtr device_ptr, WCHAR* shader_path, LPCSTR sz_entry_point, LPCSTR sz_shader_model, D3D_SHADER_MACRO* p_defines)
{
	HRESULT hr = S_OK;

	DWORD dw_shader_flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dw_shader_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	Microsoft::WRL::ComPtr<ID3DBlob> err;
	hr = D3DCompileFromFile(shader_path, p_defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, sz_entry_point, sz_shader_model, dw_shader_flags, 0, m_shader_buffer_ptr.GetAddressOf(), &err);
	if (FAILED(hr) && err)
		OutputDebugStringA(reinterpret_cast<const char*>(err->GetBufferPointer()));

	hr = device_ptr->CreatePixelShader(m_shader_buffer_ptr->GetBufferPointer(), m_shader_buffer_ptr->GetBufferSize(), NULL, m_shader_ptr.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring error_msg = L"Failed to create pixel shader";
		utils::WinErrorLogger::Log(hr, error_msg);
		return false;
	}

	return true;
}


ID3D11PixelShader* PixelShader::GetShaderPtr()
{
	return m_shader_ptr.Get();
}

ID3D10Blob* PixelShader::GetBufferPtr()
{
	return m_shader_buffer_ptr.Get();
}