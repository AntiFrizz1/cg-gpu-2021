#pragma once
#include "../tiny_gltf.h"
#include "ConstantBuffer.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <wchar.h>
#include "Shaders.h"

enum class ShaderModelCompileFlags : uint8_t
{
	NormalTexture            = 0x1,
	OclusionTexture          = 0x2,
	MetallicRoughnessTexture = 0x4,
	BaseColorTexture         = 0x8,
	EmmisiveTexture          = 0x10
};

struct Material
{
	uint8_t flags = 0;
	std::unordered_map<ShaderModelCompileFlags, size_t> textures;
	MaterialConstantBuffer material_const_buffer;
	bool alpha_mode = false;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_state_ptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state_ptr;
};


struct Primitive
{
	int world_matrix_index;
	int material_index;
	Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;
	int index_buffer_size;
	DXGI_FORMAT index_format;
	D3D11_PRIMITIVE_TOPOLOGY primitive_topology;
	DirectX::XMVECTOR max;
	DirectX::XMVECTOR min;
	std::vector<std::pair<UINT, Microsoft::WRL::ComPtr<ID3D11Buffer>>> vertex_buffers;
};

class Model
{
public:

	bool Load(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr, std::string model_filepath);
	void Render(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context_ptr,
		ConstantBuffer cb_data,
		Microsoft::WRL::ComPtr<ID3D11Buffer> cb_buffer,
		Microsoft::WRL::ComPtr < ID3D11Buffer> material_cb_buffer,
		DirectX::XMFLOAT3 camera_forward_dir);

	void SetScale(float scale);
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetPixelShaderPath(const wchar_t* path);
	void SetVertexShaderPath(const wchar_t* path);
	void SetEmissivePixelShaderPath(const wchar_t* path);
private:
	bool load_materials(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr);
	bool load_textures(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr);
	bool add_pixel_shader(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr, uint8_t flags);
	bool create_sampler_state(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr);
	bool create_primitives(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr);
	bool process_node(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr, int node, DirectX::XMMATRIX world_matrix);
	bool create_primitive(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr, tinygltf::Primitive const& gltf_primitive, UINT world_matrix_index);
	bool create_shaders(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr);
	void render_primitive(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context_ptr,
		Primitive const& primitive,
		ConstantBuffer& transformation_cb_data,
		Microsoft::WRL::ComPtr<ID3D11Buffer> transformation_cb,
		Microsoft::WRL::ComPtr<ID3D11Buffer> material_cb,
		bool emissive=false);

	void sort_and_render_transparent(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context_ptr,
		std::vector<Primitive> const& transparent_primitives,
		ConstantBuffer transformation_cb_data,
		Microsoft::WRL::ComPtr<ID3D11Buffer> transformation_cb,
		Microsoft::WRL::ComPtr<ID3D11Buffer> material_cb,
		DirectX::XMFLOAT3 camera_forward_dir_input,
		bool emissive = false);

	tinygltf::Model m_gltf_model;
	std::vector<Material> m_materials;
	std::vector< Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_shader_resource_views;
	std::unordered_map<uint8_t, PixelShader> m_pixel_shaders;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler_state_ptr;

	std::wstring m_vertex_shader_path;
	std::wstring m_pixel_shader_path;
	std::wstring m_emissive_pixel_shader_path;
	std::vector< DirectX::XMMATRIX> m_world_matricies;
	std::vector<Primitive> m_primitives;
	std::vector<Primitive> m_transparent_primitives;
	std::vector<Primitive> m_emissive_primitives;
	std::vector<Primitive> m_transparent_emissive_primitives;

	float m_scale;
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMMATRIX m_world_transformation;

	VertexShader m_vertex_shader;
	PixelShader  m_emissive_pixel_shader;
};
