#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

#include "AdapterReader.h"
#include "Shaders.h"
#include "Vertex.h"
#include "ConstantBuffer.h"
#include "CameraPosition.h"
#include "ToneMaping.h"
#include "Sphere.h"
#include "WorldCameraPosition.h"
#include "ConstantBuffer.h"

//#pragma comment (lib, "d3d11.lib")
//#pragma comment (lib, "DirectXTK.lib")

enum class PbrShaderType
{
	BRDF,
	NDF,
	GEOMETRY,
	FRESNEL
};

class Graphics
{
public:
	bool Initialize(HWND hwnd, size_t width, size_t height);
	void RenderFrame();

	DirectX::XMMATRIX& RefWorld() { return m_world1; }
	DirectX::XMMATRIX& RefView() { return m_view; }
	DirectX::XMMATRIX& RefProjection() { return m_projection; }
	CameraPosition& RefCamera() { return m_camera; }
	WorldCameraPosition& RefWorldCameraPosition() { return m_camera_position; }
	void ChangeLightsIntencity(size_t ind);
	void SetToneMaping(bool enable) { m_tone_maping_enable = enable; }
	void SwitchToneMaping() { m_tone_maping_enable ^= true; }
	void SetPbrShaderType(PbrShaderType type) { m_cur_pbr_shader_type = type; }
	bool OnResizeWindow(size_t width, size_t height);


private:
	bool initialize_directx(HWND hwnd, size_t width, size_t height);
	bool initilize_shaders();
	bool initilize_scene();
	bool initialize_tone_maping();
	bool initialize_lights();
	bool update_texture();
	bool load_texture(const char* path);

	bool create_cubemap_texture();
	bool create_cubemap_from_texture(size_t cubemap_size, ID3D11Texture2D* dst, ID3D11ShaderResourceView* src, VertexShader* vs, PixelShader* ps, UINT mip_slice);
	bool create_irradiance_texture_from_cubemap();
	bool create_depth_stencil_buffer(size_t width, size_t height);


	Microsoft::WRL::ComPtr<ID3D11Device>             m_device_ptr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>      m_device_context_ptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain>           m_swap_chain_ptr;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_render_taget_view_ptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture_resource_view;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>       m_sampler_linear;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_env_cubemap_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_env_cubemap_texture_resource_view;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_env_irradiance_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_env_irradiance_texture_resource_view;


	ToneMaping m_tone_maping;
	RenderInTexture m_render_in_texture{ DXGI_FORMAT_R32G32B32A32_FLOAT };
	RenderInTexture m_render_in_texture_for_cubemap{ DXGI_FORMAT_R32G32B32A32_FLOAT };
	VertexShader m_vertex_shader;
	PixelShader  m_env_pixel_shader;

	PixelShader m_brdf_pixel_shader;
	PixelShader m_ndf_pixel_shader;
	PixelShader m_geometry_pixel_shader;
	PixelShader m_fresnel_pixel_shader;

	VertexShader m_env_cubemap_vertex_shader;
	PixelShader m_env_cubemap_pixel_shader;

	VertexShader m_env_irradiance_vertex_shader;
	PixelShader m_env_irradiance_pixel_shader;

	PbrShaderType m_cur_pbr_shader_type{ PbrShaderType::BRDF };

	Sphere m_sphere;
	std::vector<Vertex> m_sphere_vertex;
	std::vector<WORD> m_sphere_indicies;

	DirectX::XMFLOAT4 m_vLightDirs[NUM_OF_LIGHT];
	DirectX::XMFLOAT4 m_vLightColors[NUM_OF_LIGHT];
	float             m_vLightIntencitys[NUM_OF_LIGHT];
	size_t            m_vLightIntencityIndex[NUM_OF_LIGHT];

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_index_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constant_buffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer>  m_lights_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>  m_material_buffer;

	
	//LightsConstantBuffer				m_lights_buffer_data;
	//MaterialConstantBuffer				m_material_buffer_data;

	D3D11_VIEWPORT    m_viewport;
	DirectX::XMMATRIX m_translation;
	DirectX::XMMATRIX m_world1;
	DirectX::XMMATRIX m_view;
	DirectX::XMMATRIX m_projection;
	
	CameraPosition m_camera;
	WorldCameraPosition m_camera_position;
	bool m_tone_maping_enable{ true };
	size_t m_width;
	size_t m_height;
	D3D_DRIVER_TYPE m_driver_type;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depth_ptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthDSV_ptr;
};
