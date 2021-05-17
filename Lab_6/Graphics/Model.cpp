//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "Model.h"
//#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#undef TINYGLTF_IMPLEMENTATION
#include "../WinErrorLoger.h"

bool Model::Load(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr, std::string model_filepath)
{
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&m_gltf_model, &err, &warn, model_filepath);

	if (!warn.empty())
	{
		utils::WinErrorLogger::Log("Warn: " + warn);
	}

	if (!err.empty())
	{
		utils::WinErrorLogger::Log("Err: " + err);
	}

	if (!ret)
	{
		utils::WinErrorLogger::Log("Gltf model loading failed");
		return false;
	}
	if (!create_sampler_state(device_ptr))
	{
		return false;
	}
	if (!load_materials(device_ptr))
	{
		return false;
	}
	if (!load_textures(device_ptr))
	{
		return false;
	}
	if (!create_shaders(device_ptr))
	{
		return false;
	}
	if (!create_primitives(device_ptr))
	{
		return false;
	}
}

bool Model::load_materials(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr)
{
	for (tinygltf::Material const& gltf_material : m_gltf_model.materials)
	{
		Material material;
		if (gltf_material.alphaMode == "BLEND")
		{
			material.alpha_mode = true;
			D3D11_BLEND_DESC blend_desc = {};
			blend_desc.RenderTarget[0].BlendEnable = true;
			blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
			HRESULT hr = device_ptr->CreateBlendState(&blend_desc, &material.blend_state_ptr);
			if (FAILED(hr))
			{
				utils::WinErrorLogger::Log(hr, "Failed to create blend state");
				return false;
			}
		}
		else 
		{
			material.alpha_mode = false;
			if (gltf_material.alphaMode != "OPAQUE")
			{
				std::string output_msg = "Unsupported alpha mode \"" + gltf_material.alphaMode + "\"";
				OutputDebugStringA(output_msg.c_str());
			}
		}

		auto const& base_color = gltf_material.pbrMetallicRoughness.baseColorFactor;
		material.material_const_buffer.albedo = DirectX::XMFLOAT4(base_color[0], base_color[1], base_color[2], base_color[3]);
		material.material_const_buffer.roughness = gltf_material.pbrMetallicRoughness.roughnessFactor;
		material.material_const_buffer.metalness = gltf_material.pbrMetallicRoughness.metallicFactor;

		D3D11_RASTERIZER_DESC rasterizer_desc = {};
		rasterizer_desc.FillMode = D3D11_FILL_SOLID;
		if (gltf_material.doubleSided)
			rasterizer_desc.CullMode = D3D11_CULL_NONE;
		else
			rasterizer_desc.CullMode = D3D11_CULL_BACK;
		rasterizer_desc.FrontCounterClockwise = true;
		rasterizer_desc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
		rasterizer_desc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizer_desc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		HRESULT hr = device_ptr->CreateRasterizerState(&rasterizer_desc, &material.rasterizer_state_ptr);
		if (FAILED(hr))
		{
			utils::WinErrorLogger::Log(hr, "Failed to create rasterizer state");
			return false;
		}

		material.flags = 0;
		int normal_texture_index = gltf_material.normalTexture.index;
		if (normal_texture_index >= 0)
		{
			material.flags |= static_cast<uint8_t>(ShaderModelCompileFlags::NormalTexture);
			material.textures[ShaderModelCompileFlags::NormalTexture] = normal_texture_index;
		}
		int oclussion_texture_index = gltf_material.occlusionTexture.index;
		if (oclussion_texture_index >= 0)
		{
			material.flags |= static_cast<uint8_t>(ShaderModelCompileFlags::OclusionTexture);
			material.textures[ShaderModelCompileFlags::OclusionTexture] = oclussion_texture_index;
		}
		int metallic_roghness_texture_index = gltf_material.pbrMetallicRoughness.metallicRoughnessTexture.index;
		if (metallic_roghness_texture_index >= 0)
		{
			material.flags |= static_cast<uint8_t>(ShaderModelCompileFlags::MetallicRoughnessTexture);
			material.textures[ShaderModelCompileFlags::MetallicRoughnessTexture] = metallic_roghness_texture_index;
		}
		int base_color_index = gltf_material.pbrMetallicRoughness.baseColorTexture.index;
		if (base_color_index >= 0)
		{
			material.flags |= static_cast<uint8_t>(ShaderModelCompileFlags::BaseColorTexture);
			material.textures[ShaderModelCompileFlags::BaseColorTexture] = base_color_index;
		}
		int emmisive_index = gltf_material.emissiveTexture.index;
		if (emmisive_index >= 0)
		{
			material.flags |= static_cast<uint8_t>(ShaderModelCompileFlags::EmmisiveTexture);
			material.textures[ShaderModelCompileFlags::EmmisiveTexture] = emmisive_index;
		}
		if (m_pixel_shaders.count(material.flags) == 0)
		{
			add_pixel_shader(device_ptr, material.flags);
		}
		m_materials.push_back(material);
	}
	return true;
}

bool Model::load_textures(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr)
{
	for (tinygltf::Image const& image : m_gltf_model.images)
	{
		std::string const& imagepath = image.uri;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		CD3D11_TEXTURE2D_DESC texture_desc(format, image.width, image.height, 1, 1, D3D11_BIND_SHADER_RESOURCE);
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = image.image.data();
		initData.SysMemPitch = 4 * image.width;
		HRESULT hr = device_ptr->CreateTexture2D(&texture_desc, &initData, &texture);
		if (FAILED(hr))
		{
			utils::WinErrorLogger::Log(hr, "Failed to create texture");
			return false;
		}

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
		CD3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc(D3D11_SRV_DIMENSION_TEXTURE2D, texture_desc.Format);
		hr = device_ptr->CreateShaderResourceView(texture.Get(), &shader_resource_view_desc, &shader_resource_view);
		if (FAILED(hr))
		{
			utils::WinErrorLogger::Log(hr, "Failed to create shader resource view");
			return false;
		}
		m_shader_resource_views.push_back(shader_resource_view);
	}

}

bool Model::add_pixel_shader(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr, uint8_t flags)
{
	std::vector<D3D_SHADER_MACRO> defines;
	if (flags & static_cast<uint8_t>(ShaderModelCompileFlags::BaseColorTexture))         defines.push_back({ "BASE_COLOR_TEXTURE", "1" });
	if (flags & static_cast<uint8_t>(ShaderModelCompileFlags::MetallicRoughnessTexture)) defines.push_back({ "METALLIC_ROUGHNESS_TEXTURE", "1" });
	if (flags & static_cast<uint8_t>(ShaderModelCompileFlags::NormalTexture))            defines.push_back({ "NORMAL_TEXTURE", "1" });
	if (flags & static_cast<uint8_t>(ShaderModelCompileFlags::OclusionTexture))          defines.push_back({ "OCLUSION_TEXTURE", "1" });

	defines.push_back({ nullptr, nullptr });

	PixelShader pixel_shader;
	if (!pixel_shader.Initialize(device_ptr, m_pixel_shader_path.c_str(), "main", "ps_5_0", defines.data()))
	{
		utils::WinErrorLogger::Log("Failed to create pixel shader");
		return false;
	}
	m_pixel_shaders.emplace(flags, pixel_shader);

	return true;
}

bool Model::create_sampler_state(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr) {
	
	tinygltf::Sampler const& gltf_sampler = m_gltf_model.samplers[0];
	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));

	switch (gltf_sampler.minFilter)
	{
	case TINYGLTF_TEXTURE_FILTER_NEAREST:
		if (gltf_sampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		}
		else
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}
		break;
	case TINYGLTF_TEXTURE_FILTER_LINEAR:
		if (gltf_sampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		}
		else
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		}
		break;
	case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
		if (gltf_sampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		}
		else
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		}
		break;
	case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
		if (gltf_sampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		}
		else
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		}
		break;
	case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
		if (gltf_sampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		}
		else
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		}
		break;
	case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
		if (gltf_sampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		}
		else
		{
			sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}
		break;
	default:
		sampler_desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	}

	switch (gltf_sampler.wrapS)
	{
	case TINYGLTF_TEXTURE_WRAP_REPEAT:
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
		sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
		break;
	default:
		sampler_desc.AddressU =  D3D11_TEXTURE_ADDRESS_WRAP;
	}

	switch (gltf_sampler.wrapT)
	{
	case TINYGLTF_TEXTURE_WRAP_REPEAT:
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
		break;
	default:
		sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	}

	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	HRESULT hr = device_ptr->CreateSamplerState(&sampler_desc, m_sampler_state_ptr.GetAddressOf());
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create sampler state");
		return false;
	}

	return true;
}

bool Model::create_primitives(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr)
{
	tinygltf::Scene const& gltf_scene = m_gltf_model.scenes[m_gltf_model.defaultScene];

	m_world_matricies.push_back(DirectX::XMMatrixIdentity());


	for (int node : gltf_scene.nodes)
	{
		if (!process_node(device_ptr, node, m_world_matricies[0]))
			return false;
	}

	return true;
}

bool Model::create_primitive(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr, tinygltf::Primitive const& gltf_primitive, UINT world_matrix_index)
{
	Primitive primitive;
	primitive.world_matrix_index = world_matrix_index;

	for (auto const& [ name, value ] : gltf_primitive.attributes)
	{
		if (name == "TEXCOORD_1") // It isn't used in model
			continue;

		tinygltf::Accessor const& gltf_accessor = m_gltf_model.accessors[value];
		tinygltf::BufferView& gltf_buffer_view = m_gltf_model.bufferViews[gltf_accessor.bufferView];
		tinygltf::Buffer& gltf_buffer = m_gltf_model.buffers[gltf_buffer_view.buffer];

		UINT byte_stride = static_cast<UINT>(gltf_accessor.ByteStride(gltf_buffer_view));
		CD3D11_BUFFER_DESC buffer_desc(byte_stride * static_cast<UINT>(gltf_accessor.count), D3D11_BIND_VERTEX_BUFFER);
		buffer_desc.StructureByteStride = byte_stride;

		D3D11_SUBRESOURCE_DATA init_data;
		ZeroMemory(&init_data, sizeof(D3D11_SUBRESOURCE_DATA));
		init_data.pSysMem = &gltf_buffer.data[gltf_buffer_view.byteOffset + gltf_accessor.byteOffset];
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer_ptr;
		HRESULT hr = device_ptr->CreateBuffer(&buffer_desc, &init_data, &vertex_buffer_ptr);
		if (FAILED(hr))
		{
			utils::WinErrorLogger::Log(hr, "Failed to create buffer");
			return false;
		}

		primitive.vertex_buffers.emplace_back(byte_stride, vertex_buffer_ptr);

		if (name == "POSITION")
		{
			DirectX::XMFLOAT3 maxPosition(static_cast<float>(gltf_accessor.maxValues[0]), static_cast<float>(gltf_accessor.maxValues[1]), static_cast<float>(gltf_accessor.maxValues[2]));
			DirectX::XMFLOAT3 minPosition(static_cast<float>(gltf_accessor.minValues[0]), static_cast<float>(gltf_accessor.minValues[1]), static_cast<float>(gltf_accessor.minValues[2]));

			primitive.max = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&maxPosition), m_world_matricies[primitive.world_matrix_index]);
			primitive.min = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&minPosition), m_world_matricies[primitive.world_matrix_index]);
		}
	}

	switch (gltf_primitive.mode)
	{
	case TINYGLTF_MODE_POINTS:
		primitive.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		break;
	case TINYGLTF_MODE_LINE:
		primitive.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	case TINYGLTF_MODE_LINE_STRIP:
		primitive.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		break;
	case TINYGLTF_MODE_TRIANGLES:
		primitive.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	case TINYGLTF_MODE_TRIANGLE_STRIP:
		primitive.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		break;
	}

	tinygltf::Accessor& gltf_accessor = m_gltf_model.accessors[gltf_primitive.indices];
	tinygltf::BufferView& gltf_buffer_view = m_gltf_model.bufferViews[gltf_accessor.bufferView];
	tinygltf::Buffer& gltf_buffer = m_gltf_model.buffers[gltf_buffer_view.buffer];

	primitive.index_buffer_size = static_cast<uint32_t>(gltf_accessor.count);
	UINT stride = 2;
	switch (gltf_accessor.componentType)
	{
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
		primitive.index_format = DXGI_FORMAT_R8_UINT;
		stride = 1;
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
		primitive.index_format = DXGI_FORMAT_R16_UINT;
		stride = 2;
		break;
	case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
		primitive.index_format = DXGI_FORMAT_R32_UINT;
		stride = 4;
		break;
	}

	CD3D11_BUFFER_DESC buffer_desc(stride * primitive.index_buffer_size, D3D11_BIND_INDEX_BUFFER);
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	initData.pSysMem = &gltf_buffer.data[gltf_buffer_view.byteOffset + gltf_accessor.byteOffset];
	HRESULT hr = device_ptr->CreateBuffer(&buffer_desc, &initData, &primitive.index_buffer);
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create buffer");
		return false;
	}

	primitive.material_index = gltf_primitive.material;
	if (m_materials[primitive.material_index].alpha_mode)
	{
		m_transparent_primitives.push_back(primitive);
		if (m_materials[primitive.material_index].textures.count(ShaderModelCompileFlags::EmmisiveTexture) > 0)
		{
			m_transparent_emissive_primitives.push_back(primitive);
		}
	}
	else
	{
		m_primitives.push_back(primitive);
		if (m_materials[primitive.material_index].textures.count(ShaderModelCompileFlags::EmmisiveTexture) > 0)
		{
			m_emissive_primitives.push_back(primitive);
		}
	}

	return true;
}

bool Model::process_node(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr, int node, DirectX::XMMATRIX world_matrix)
{
	tinygltf::Node const& gltf_node = m_gltf_model.nodes[node];

	if (gltf_node.mesh >= 0)
	{
		tinygltf::Mesh const& gltf_mesh = m_gltf_model.meshes[gltf_node.mesh];
		UINT matrix_index = static_cast<UINT>(m_world_matricies.size() - 1);

		for (tinygltf::Primitive const& gltf_primitive : gltf_mesh.primitives)
		{
			if (!create_primitive(device_ptr, gltf_primitive, matrix_index))
				return false;
		}
	}

	if (gltf_node.children.size() > 0)
	{
		DirectX::XMMATRIX rotation;
		if (gltf_node.matrix.empty())
		{
			if (gltf_node.rotation.empty())
				rotation = DirectX::XMMatrixIdentity();
			else
			{
				float v[4] = {};
				float* p = v;
				for (double value : gltf_node.rotation)
				{
					*p = static_cast<float>(value);
					++p;
				}
				DirectX::XMFLOAT4 vector(v);
				rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&vector));
			}
		}
		else
		{
			float flat[16] = {};
			float* p = flat;
			for (double value : gltf_node.matrix)
			{
				*p = static_cast<float>(value);
				++p;
			}
			DirectX::XMFLOAT4X4 matrix(flat);
			rotation = DirectX::XMLoadFloat4x4(&matrix);
		}

		world_matrix = DirectX::XMMatrixMultiplyTranspose(DirectX::XMMatrixTranspose(world_matrix), DirectX::XMMatrixTranspose(rotation));
		m_world_matricies.push_back(world_matrix);
		for (int child_node : gltf_node.children)
		{
			if (!process_node(device_ptr, child_node, world_matrix))
				return false;
		}
	}

	return true;
}

bool Model::create_shaders(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr)
{

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD_", 0, DXGI_FORMAT_R32G32_FLOAT, 3, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT num_elements = ARRAYSIZE(layout);
	if (!m_vertex_shader.Initialize(device_ptr, m_vertex_shader_path, layout, num_elements))
	{
		return false;
	}

	if (!m_emissive_pixel_shader.Initialize(device_ptr, m_emissive_pixel_shader_path))
	{
		return false;
	}
	return true;
}

void Model::Render(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context_ptr, 
	               ConstantBuffer transformation_cb_data,
	               Microsoft::WRL::ComPtr<ID3D11Buffer> transformation_cb,
				   Microsoft::WRL::ComPtr<ID3D11Buffer> material_cb,
	               DirectX::XMFLOAT3 camera_forward_dir)
{
	m_world_transformation = DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_rotation.x);
	m_world_transformation *= DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_rotation.y);
	m_world_transformation *= DirectX::XMMatrixRotationAxis({ 0,0,1 }, m_rotation.z);
	m_world_transformation *= DirectX::XMMatrixScaling(m_scale, m_scale, m_scale);
	m_world_transformation *= DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);


	device_context_ptr->VSSetConstantBuffers(0, 1, transformation_cb.GetAddressOf());
	device_context_ptr->PSSetConstantBuffers(0, 1, transformation_cb.GetAddressOf());
	device_context_ptr->PSSetConstantBuffers(2, 1, material_cb.GetAddressOf());
	device_context_ptr->PSSetSamplers(2, 1, m_sampler_state_ptr.GetAddressOf());

	transformation_cb_data.world = DirectX::XMMatrixIdentity();
	for (Primitive& primitive : m_primitives)
	{
		render_primitive(device_context_ptr, primitive, transformation_cb_data, transformation_cb, material_cb);
	}

	sort_and_render_transparent(device_context_ptr, m_transparent_primitives, transformation_cb_data, transformation_cb, material_cb, camera_forward_dir);

	for (Primitive& primitive : m_emissive_primitives)
	{
		render_primitive(device_context_ptr, primitive, transformation_cb_data, transformation_cb, material_cb, true);
	}

	sort_and_render_transparent(device_context_ptr, m_transparent_emissive_primitives, transformation_cb_data, transformation_cb, material_cb, camera_forward_dir, true);

}

void Model::sort_and_render_transparent(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context_ptr, 
	                                    std::vector<Primitive> const& transparent_primitives, 
	                                    ConstantBuffer transformation_cb_data, 
	                                    Microsoft::WRL::ComPtr<ID3D11Buffer> transformation_cb, 
										Microsoft::WRL::ComPtr<ID3D11Buffer> material_cb, 
	                                    DirectX::XMFLOAT3 camera_forward_dir_input,
	                                    bool emissive)
{
	DirectX::XMVECTOR camera_forward_dir = DirectX::XMLoadFloat3(&camera_forward_dir_input);
	std::vector<std::pair<float, size_t>> distances;
	float distance;
	DirectX::XMVECTOR center;
	DirectX::XMVECTOR cameraPos = DirectX::XMLoadFloat4(&transformation_cb_data.eye);
	for (size_t i = 0; i < transparent_primitives.size(); ++i)
	{
		center = DirectX::XMVectorDivide(DirectX::XMVectorAdd(transparent_primitives[i].max, transparent_primitives[i].min), DirectX::XMVectorReplicate(2));
		distance = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMVectorSubtract(center, cameraPos), camera_forward_dir));
		distances.push_back(std::pair<float, size_t>(distance, i));
	}

	std::sort(distances.begin(), distances.end(), [](std::pair<float, size_t> const& p1, std::pair<float, size_t> const& p2) { return p1.first < p2.first; });

	for (auto iter = distances.rbegin(); iter != distances.rend(); ++iter)
	{
		render_primitive(device_context_ptr, transparent_primitives[(*iter).second], transformation_cb_data, transformation_cb, material_cb, emissive);
	}
}

void Model::render_primitive(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context_ptr, 
	                         Primitive const& primitive, 
	                         ConstantBuffer& transformation_cb_data, 
							 Microsoft::WRL::ComPtr<ID3D11Buffer> transformation_cb,
							 Microsoft::WRL::ComPtr<ID3D11Buffer> material_cb,
	                         bool emissive)
{
	std::vector<ID3D11Buffer*> combined;
	std::vector<UINT> offset;
	std::vector<UINT> stride;

	size_t attributes_count = primitive.vertex_buffers.size();
	combined.resize(attributes_count);
	offset.resize(attributes_count);
	stride.resize(attributes_count);
	for (size_t i = 0; i < attributes_count; ++i)
	{
		combined[i] = primitive.vertex_buffers[i].second.Get();
		stride[i] = primitive.vertex_buffers[i].first;
	}
	device_context_ptr->IASetVertexBuffers(0, static_cast<UINT>(attributes_count), combined.data(), stride.data(), offset.data());

	device_context_ptr->IASetIndexBuffer(primitive.index_buffer.Get(), primitive.index_format, 0);
	device_context_ptr->IASetPrimitiveTopology(primitive.primitive_topology);

	Material const& material = m_materials[primitive.material_index];
	if (material.alpha_mode)
	{
		device_context_ptr->OMSetBlendState(material.blend_state_ptr.Get(), nullptr, 0xFFFFFFFF);
	}
	device_context_ptr->RSSetState(material.rasterizer_state_ptr.Get());

	device_context_ptr->IASetInputLayout(m_vertex_shader.GetInputLayoutPtr());
	device_context_ptr->VSSetShader(m_vertex_shader.GetShaderPtr(), nullptr, 0);

	transformation_cb_data.world = DirectX::XMMatrixTranspose(m_world_matricies[primitive.world_matrix_index] * m_world_transformation);

	device_context_ptr->UpdateSubresource(transformation_cb.Get(), 0, NULL, &transformation_cb_data, 0, 0);

	MaterialConstantBuffer material_buffer_data = material.material_const_buffer;
	if (emissive)
	{
		material_buffer_data.albedo = DirectX::XMFLOAT4(1, 1, 1, material_buffer_data.albedo.w);
		device_context_ptr->PSSetShaderResources(3, 1, m_shader_resource_views[material.textures.at(ShaderModelCompileFlags::EmmisiveTexture)].GetAddressOf());
		device_context_ptr->PSSetShader(m_emissive_pixel_shader.GetShaderPtr(), nullptr, 0);
	}
	else
	{
		device_context_ptr->PSSetShader(m_pixel_shaders[material.flags].GetShaderPtr(), nullptr, 0);
		if (material.textures.count(ShaderModelCompileFlags::BaseColorTexture) > 0)
		{
			device_context_ptr->PSSetShaderResources(3, 1, m_shader_resource_views[material.textures.at(ShaderModelCompileFlags::BaseColorTexture)].GetAddressOf());
		}
		if (material.textures.count(ShaderModelCompileFlags::MetallicRoughnessTexture) > 0)
		{
			device_context_ptr->PSSetShaderResources(4, 1, m_shader_resource_views[material.textures.at(ShaderModelCompileFlags::MetallicRoughnessTexture)].GetAddressOf());
		}
		if (material.textures.count(ShaderModelCompileFlags::NormalTexture) > 0)
		{
			device_context_ptr->PSSetShaderResources(5, 1, m_shader_resource_views[material.textures.at(ShaderModelCompileFlags::NormalTexture)].GetAddressOf());
		}
		if (material.textures.count(ShaderModelCompileFlags::OclusionTexture) > 0)
		{
			device_context_ptr->PSSetShaderResources(6, 1, m_shader_resource_views[material.textures.at(ShaderModelCompileFlags::OclusionTexture)].GetAddressOf());
		}
	}
	device_context_ptr->UpdateSubresource(material_cb.Get(), 0, NULL, &material.material_const_buffer, 0, 0);

	device_context_ptr->DrawIndexed(primitive.index_buffer_size, 0, 0);

	if (material.alpha_mode)
		device_context_ptr->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

void Model::SetPixelShaderPath(const wchar_t* path)
{
	m_pixel_shader_path = path;
}
void Model::SetVertexShaderPath(const wchar_t* path)
{
	m_vertex_shader_path = path;
}

void Model::SetEmissivePixelShaderPath(const wchar_t* path)
{
	m_emissive_pixel_shader_path = path;
}

void Model::SetScale(float scale)
{
	m_scale = scale;
}

void Model::SetPosition(float x, float y, float z)
{
	m_position = DirectX::XMFLOAT3(x, y, z);
}

void Model::SetRotation(float x, float y, float z)
{
	m_rotation = DirectX::XMFLOAT3(x, y, z);
}
