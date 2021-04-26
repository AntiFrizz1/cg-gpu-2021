#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>

#include "AverageLuminanceCalculator.h"
#include "Shaders.h"

class ToneMaping
{	
public:
	
	bool Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device,
					Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context,
					size_t width,
					size_t height);

	void Process(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
			     Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sourceTexture, 
				 Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget, 
		         D3D11_VIEWPORT viewport);
	bool OnResizeWindow(size_t width, size_t height);

private:
	std::unique_ptr<AverageLuminanceCalculator> m_average_luminance_calculator;

	Microsoft::WRL::ComPtr<ID3D11SamplerState>       m_tone_sampler_state;
	Microsoft::WRL::ComPtr<ID3D11Buffer>             m_luminance_buffer;

	VertexShader m_tone_vertex_shader;
	PixelShader  m_tone_pixel_shader;

	size_t m_width;
	size_t m_height;
};