#include "ToneMaping.h"
#include "ConstantBuffer.h"
#include "../Global.h"

bool ToneMaping::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device,
							Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context,
							size_t width,
							size_t height)
{
	m_width = width;
	m_height = height;

	if (!m_tone_vertex_shader.Initialize(device, L"tone_vertex_shader.cso", NULL, 0))
	{
		return false;
	}

	if (!m_tone_pixel_shader.Initialize(device, L"tone_pixel_shader.cso"))
	{
		return false;
	}

	// Create the sampler state
	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;
	sd.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	HRESULT hr = device->CreateSamplerState(&sd, m_tone_sampler_state.GetAddressOf());
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create smapler state");
		return false;
	}
	m_average_luminance_calculator = std::make_unique<AverageLuminanceCalculator>();
	if (!m_average_luminance_calculator->Initialize(device, width, height))
	{
		return false;
	}

	CD3D11_BUFFER_DESC tbd(sizeof(LuminanceConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
	hr = device->CreateBuffer(&tbd, nullptr, m_luminance_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create constant time buffer.");
		return false;
	}

	return true;
}

void ToneMaping::Process(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
						 Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sourceTexture,
						 Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget,
						 D3D11_VIEWPORT viewport)
{
	Global::GetAnnotation().BeginEvent(L"Start ToneMaping.");
	float averageLuminance = m_average_luminance_calculator->process(context, sourceTexture);

	LuminanceConstantBuffer luminanceBufferData = { averageLuminance };

	context->UpdateSubresource(m_luminance_buffer.Get(), 0, nullptr, &luminanceBufferData, 0, 0);
	context->PSSetShaderResources(0, 1, sourceTexture.GetAddressOf());
	context->OMSetRenderTargets(1, renderTarget.GetAddressOf(), nullptr);
	context->RSSetViewports(1, &viewport);

	context->IASetInputLayout(nullptr);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	context->VSSetShader(m_tone_vertex_shader.GetShaderPtr(), nullptr, 0);
	context->PSSetShader(m_tone_pixel_shader.GetShaderPtr(), nullptr, 0);
	context->PSSetConstantBuffers(0, 1, m_luminance_buffer.GetAddressOf());
	context->PSSetSamplers(0, 1, m_tone_sampler_state.GetAddressOf());

	context->Draw(4, 0);

	ID3D11ShaderResourceView* nullsrv[] = { nullptr };
	context->PSSetShaderResources(0, 1, nullsrv);
	Global::GetAnnotation().EndEvent();
}

bool ToneMaping::OnResizeWindow(size_t width, size_t height)
{
	m_width = width;
	m_height = height;
	return m_average_luminance_calculator->OnResizeWindow(width, height);
}
