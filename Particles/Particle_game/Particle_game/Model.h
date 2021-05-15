#pragma once
#include "tiny_gltf.h"
#include "Graphics/Shaders.h"
#include "Graphics/ConstantBuffer.h"
#include <d3d11.h>

class Model
{
public:
	Model(const char* path);
	~Model();
	bool Load(ID3D11Device* device);

	void SetPixelShaderPath(const wchar_t* path);
	void SetVertexShaderPath(const wchar_t* path);
	void SetScale(float scale);
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);

	struct ShadersSlots
	{
		UINT base_color_texture_slot;
		UINT metallic_roughness_texture_slot;
		UINT normal_texture_slot;
		UINT sampler_state_slot;
		UINT transformation_cb_slot;
		UINT material_cb_slot;
	};

	enum MODEL_PIXEL_SHADER_DEFINES
	{
		MATERIAL_HAS_COLOR_TEXTURE = 0x1,
		MATERIAL_HAS_METAL_ROUGH_TEXTURE = 0x2,
		MATERIAL_HAS_NORMAL_TEXTURE = 0x4,
		MATERIAL_HAS_OCCLUSION_TEXTURE = 0x8
	} MODEL_PIXEL_SHADER_DEFINES;

	void Render(ID3D11DeviceContext* context, ConstantBuffer transformation_cb_data, ID3D11Buffer* transformation_cb, ID3D11Buffer* material_cb, ShadersSlots slots, DirectX::XMFLOAT3 camera_forward_dir);
	private:

	struct Material
	{
		std::string name;
		bool blend;
		Microsoft::WRL::ComPtr<ID3D11BlendState> blend_state_ptr;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state_ptr;
		MaterialConstantBuffer material_buffer_data;
		int base_color_texture;
		int metallic_roughness_texture;
		int normal_texture;
		int emissive_texture;
		UINT ps_defines_flags;
	};

	struct Attribute
	{
		UINT byteStride;
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_ptr;
	};

	struct Primitive
	{
		std::vector<Attribute> attributes;
		UINT vertex_count;
		DirectX::XMVECTOR max;
		DirectX::XMVECTOR min;
		D3D11_PRIMITIVE_TOPOLOGY primitive_topology;
		DXGI_FORMAT indexFormat;
		Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_ptr;
		UINT index_count;
		UINT material;
		UINT matrix;
	};

	bool create_texture(ID3D11Device* device, size_t imageIdx, bool useSRGB = false);

	bool create_sampler_state(ID3D11Device* device);
	bool create_materials(ID3D11Device* device);

	bool create_primitives(ID3D11Device* device);
	bool process_node(ID3D11Device* device, int node, DirectX::XMMATRIX world_matrix);
	bool create_primitive(ID3D11Device* device, tinygltf::Primitive& gltfPrimitive, UINT matrix);

	bool create_shaders(ID3D11Device* device);
	bool create_pixel_shader(ID3D11Device* device, UINT definesFlags);
	void sort_and_render_transparent(std::vector<Primitive> transparent_primitives, ID3D11DeviceContext* context, ConstantBuffer transformation_cb_data, ID3D11Buffer* transformation_cb, ID3D11Buffer* material_cb, ShadersSlots slots, DirectX::XMFLOAT3 camera_forward_dir_input, bool emissive);


	void render_primitive(Primitive& primitive, ID3D11DeviceContext* context, ConstantBuffer& transformationData, ID3D11Buffer* transformationConstantBuffer, ID3D11Buffer* materialConstantBuffer, ShadersSlots& slots, bool emissive = false);

	float m_scale;
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMMATRIX m_world_transformation;

	std::string m_model_path;
	tinygltf::Model m_model;
	std::wstring m_vertex_shader_path;
	std::wstring m_pixel_shader_path;
	PixelShader m_pixel_shader;
	VertexShader m_vertex_shader;


	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler_state_ptr;

	std::vector< Microsoft::WRL::ComPtr<ID3D11PixelShader>> m_pixel_shader_ptrs;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_shader_resource_view_ptrs;
	std::vector<DirectX::XMMATRIX> m_world_matricies;
	std::vector<Material>  m_materials;
	std::vector<Primitive> m_primitives;
	std::vector<Primitive> m_transparent_primitives;

	std::vector<Primitive> m_emissive_primitives;
	std::vector<Primitive> m_emissive_transparent_primitives;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>  m_pInputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  m_pEmissivePixelShader;


};

