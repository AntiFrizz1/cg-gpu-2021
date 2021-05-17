#pragma once

#include "../WinErrorLoger.h"

#pragma comment(lib, "D3DCompiler.lib")
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>


class VertexShader
{
public:
	using DeviceComPtr = Microsoft::WRL::ComPtr<ID3D11Device>;
	bool Initialize(DeviceComPtr device_ptr, std::wstring shaderpath, D3D11_INPUT_ELEMENT_DESC* layout_desc, UINT num_elements);
	bool Initialize(DeviceComPtr device_ptr, WCHAR* shaderpath, D3D11_INPUT_ELEMENT_DESC* layout_desc, UINT num_elements, LPCSTR sz_entry_point, LPCSTR sz_shader_model, D3D_SHADER_MACRO* p_defines);
	ID3D11VertexShader* GetShaderPtr();
	ID3D10Blob* GetBufferPtr();
	ID3D11InputLayout* GetInputLayoutPtr() { return m_input_layout_ptr.Get();  }
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_shader_ptr;
	Microsoft::WRL::ComPtr<ID3D10Blob>         m_shader_buffer_ptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_input_layout_ptr;
};



class PixelShader
{
public:
	using DeviceComPtr = Microsoft::WRL::ComPtr<ID3D11Device>;
	bool Initialize(DeviceComPtr device_ptr, std::wstring shaderpath);
	bool Initialize(DeviceComPtr device_ptr, WCHAR const* shader_path, LPCSTR sz_entry_point, LPCSTR sz_shader_model, D3D_SHADER_MACRO* p_defines);
	ID3D11PixelShader* GetShaderPtr();
	ID3D10Blob* GetBufferPtr();
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetShaderSharedPtr() { return m_shader_ptr;        }
	Microsoft::WRL::ComPtr<ID3D10Blob>        GetBufferSharedPtr() { return m_shader_buffer_ptr; }
private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_shader_ptr;
	Microsoft::WRL::ComPtr<ID3D10Blob>         m_shader_buffer_ptr;
};