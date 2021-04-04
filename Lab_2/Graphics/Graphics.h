#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include "AdapterReader.h"
#include "Shaders.h"
#include "Vertex.h"
#include "ConstantBuffer.h"
#include "CameraPosition.h"
#include "ToneMaping.h"

//#pragma comment (lib, "d3d11.lib")
//#pragma comment (lib, "DirectXTK.lib")

class Graphics
{
public:
	bool Initialize(HWND hwnd, size_t width, size_t height);
	void RenderFrame();

	DirectX::XMMATRIX& RefWorld() { return m_world1; }
	DirectX::XMMATRIX& RefView() { return m_view; }
	DirectX::XMMATRIX& RefProjection() { return m_projection; }
	DirectX::XMFLOAT4& RefPos() { return m_pos; }
	CameraPosition& RefCamera() { return m_camera; }
	
	void ChangeLightsIntencity(size_t ind);
	void ChangeToneMaping() { m_tone_maping_enable ^= true; }

	bool OnResizeWindow(size_t width, size_t height);
private:
	bool initialize_directx(HWND hwnd, size_t width, size_t height);
	bool initilize_shaders();
	bool initilize_scene();
	bool initialize_tone_maping();
	bool initialize_lights();
	bool update_texture();

	Microsoft::WRL::ComPtr<ID3D11Device>           m_device_ptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>    m_device_context_ptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain>         m_swap_chain_ptr;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_render_taget_view_ptr;

	ToneMaping m_tone_maping;
	RenderInTexture m_render_in_texture{ DXGI_FORMAT_R32G32B32A32_FLOAT };
	VertexShader m_vertex_shader;
	PixelShader  m_pixel_shader;



	DirectX::XMFLOAT4 m_vLightDirs[NUM_OF_LIGHT];
	DirectX::XMFLOAT4 m_vLightColors[NUM_OF_LIGHT];
	float             m_vLightIntencitys[NUM_OF_LIGHT];

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_index_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constant_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constant_buffer1;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depth_stencil_buffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depth_stencil_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depth_stencil_state;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizer_state;
	D3D11_VIEWPORT       m_viewport;
	DirectX::XMMATRIX m_translation;
	DirectX::XMMATRIX m_world1;
	DirectX::XMMATRIX m_world2;
	DirectX::XMMATRIX m_view;
	DirectX::XMMATRIX m_projection;
	DirectX::XMFLOAT4 m_pos;
	CameraPosition m_camera;
	bool m_tone_maping_enable{ true };
	size_t m_width;
	size_t m_height;
	D3D_DRIVER_TYPE m_driver_type;
};
