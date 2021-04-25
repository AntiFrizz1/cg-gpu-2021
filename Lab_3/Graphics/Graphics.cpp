#include "Graphics.h"
#include "../WinErrorLoger.h"
#include "../Global.h"
#include "DDSTextureLoader.h"

bool Graphics::Initialize(HWND hwnd, size_t width, size_t height)
{
	m_width = width;
	m_height = height;

	if (!initialize_directx(hwnd, width, height))
	{
		return false;
	}

	if (!initilize_shaders())
	{
		return false;
	}

	if (!initilize_scene())
	{
		return false;
	}

	if (!m_sphere.Initialize(m_device_ptr))
	{
		return false;
	}

	if (!m_tone_maping.Initialize(m_device_ptr, m_device_context_ptr, m_width, m_height))
	{
		return false;
	}

	if (!initialize_lights())
	{
		return false;
	}

	if (!m_render_in_texture.Initialize(m_device_ptr, m_width, m_height))
	{
		return false;
	}

	if (!create_depth_stencil_buffer(m_width, m_height)) {
		return false;
	}

	return true;
}

void Graphics::RenderFrame()
{
	Global::GetAnnotation().BeginEvent(L"Start Render.");
	float bgcolor[] = {0.0f, 0.0f, 1.0f, 1.0f};
	ID3D11RenderTargetView* render_target = m_render_in_texture.GetTextureRenderTargetView();
	D3D11_VIEWPORT viewport = m_render_in_texture.GetViewPort();
	m_device_context_ptr->ClearRenderTargetView(render_target, bgcolor);
	m_device_context_ptr->ClearRenderTargetView(m_render_taget_view_ptr.Get(), bgcolor);
	m_device_context_ptr->ClearDepthStencilView(m_depthDSV_ptr.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	if (m_tone_maping_enable)
	{
		m_device_context_ptr->OMSetRenderTargets(1, &render_target, m_depthDSV_ptr.Get());
	}
	else
	{
		m_device_context_ptr->OMSetRenderTargets(1, m_render_taget_view_ptr.GetAddressOf(), m_depthDSV_ptr.Get());
	}


	m_device_context_ptr->RSSetViewports(1, &viewport);
	m_device_context_ptr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ConstantBuffer cb;
	cb.world = DirectX::XMMatrixTranspose(m_world1);
	cb.view = DirectX::XMMatrixTranspose(m_view);
	cb.projection = DirectX::XMMatrixTranspose(m_projection);
	for (int m = 0; m < NUM_OF_LIGHT; m++) {
		cb.vLightDir[m] = m_vLightDirs[m];
		cb.vLightColor[m] = m_vLightColors[m];
		cb.vLightIntencity[m] = DirectX::XMFLOAT4(m_vLightIntencitys[m], 0.0f, 0.0f, 0.0f);
	}
	cb.eye = DirectX::XMFLOAT4(m_camera_position.pos_x, m_camera_position.pos_y, m_camera_position.pos_z, 0);

	m_device_context_ptr->UpdateSubresource(m_constant_buffer.Get(), 0, nullptr, &cb, 0, 0);
	m_device_context_ptr->IASetInputLayout(m_vertex_shader.GetInputLayoutPtr());

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	m_device_context_ptr->IASetVertexBuffers(0, 1, m_vertex_buffer.GetAddressOf(), &stride, &offset);
	m_device_context_ptr->IASetIndexBuffer(m_index_buffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	m_device_context_ptr->VSSetShader(m_vertex_shader.GetShaderPtr(), NULL, 0);
	m_device_context_ptr->VSSetConstantBuffers(0, 1, m_constant_buffer.GetAddressOf());
	m_device_context_ptr->PSSetShader(m_env_pixel_shader.GetShaderPtr(), NULL, 0);
	m_device_context_ptr->PSSetConstantBuffers(0, 1, m_constant_buffer.GetAddressOf());
	m_device_context_ptr->PSSetShaderResources(0, 1, m_texture_resource_view.GetAddressOf());
	m_device_context_ptr->PSSetSamplers(0, 1, m_sampler_linear.GetAddressOf());
	m_device_context_ptr->DrawIndexed(m_sphere_indicies.size(), 0, 0);

	switch (m_cur_pbr_shader_type)
	{
	case PbrShaderType::BRDF:
		m_device_context_ptr->PSSetShader(m_brdf_pixel_shader.GetShaderPtr(), NULL, 0);
		break;
	case PbrShaderType::NDF:
		m_device_context_ptr->PSSetShader(m_ndf_pixel_shader.GetShaderPtr(), NULL, 0);
		break;
	case PbrShaderType::GEOMETRY:
		m_device_context_ptr->PSSetShader(m_geometry_pixel_shader.GetShaderPtr(), NULL, 0);
		break;
	case PbrShaderType::FRESNEL:
		m_device_context_ptr->PSSetShader(m_fresnel_pixel_shader.GetShaderPtr(), NULL, 0);
	}

	int constexpr SPHERES_COUNT = 9;
	for (int i = 0; i < SPHERES_COUNT; ++i)
	{
		for (int j = 0; j < SPHERES_COUNT; ++j)
		{
			cb.world = DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(
				(i - SPHERES_COUNT / 2) * Sphere::RADIUS * 3,
				(j - SPHERES_COUNT / 2) * Sphere::RADIUS * 3,
				0
			));
			cb.metalness = static_cast<float>(j) / (SPHERES_COUNT - 1);
			cb.roughness = static_cast<float>(i) / (SPHERES_COUNT - 1);
			static const DirectX::XMFLOAT4 default_albedo = {0.95f, 0.64f, 0.54f, 1.0f};  // copper color
			cb.albedo = default_albedo;
			m_device_context_ptr->IASetVertexBuffers(0, 1, m_sphere.GetAddressOfVertexBuffer(), &stride, &offset);
			m_device_context_ptr->IASetIndexBuffer(m_sphere.GetIndexBuffer(), DXGI_FORMAT_R16_UINT, 0);
			m_device_context_ptr->UpdateSubresource(m_constant_buffer.Get(), 0, nullptr, &cb, 0, 0);
			m_device_context_ptr->DrawIndexed(m_sphere.GetIniciesSize(), 0, 0);
		}
	}
	
	m_viewport.Width = static_cast<FLOAT>(m_width);
	m_viewport.Height = static_cast<FLOAT>(m_height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	if (m_tone_maping_enable)
	{
		m_tone_maping.Process(m_device_context_ptr, m_render_in_texture.GetTextureShaderResourceView(), m_render_taget_view_ptr, m_viewport);
	}
	m_swap_chain_ptr->Present(1, 0);
	Global::GetAnnotation().EndEvent();
}

void Graphics::ChangeLightsIntencity(size_t ind)
{
	static constexpr float intencity_sequence[] = {1.0f, 10.0f, 100.0f, 0.0f, 0.5f};
	static constexpr size_t INTENCITY_SEQUENCE_SIZE = sizeof(intencity_sequence) / sizeof(float);

	if (ind < NUM_OF_LIGHT)
	{
		++m_vLightIntencityIndex[ind];
		m_vLightIntencityIndex[ind] %= INTENCITY_SEQUENCE_SIZE;
		m_vLightIntencitys[ind] = intencity_sequence[m_vLightIntencityIndex[ind]];
	}
}

bool Graphics::initialize_directx(HWND hwnd, size_t width, size_t height)
{ 
	std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

	if (adapters.size() < 1)
	{
		utils::WinErrorLogger::Log("No IDXGI Adapters found");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));

	scd.BufferDesc.Width = 0;
	scd.BufferDesc.Height = 0;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 2;
	scd.OutputWindow = hwnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	
	// scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	D3D_FEATURE_LEVEL feature_levels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT num_feature_levels = ARRAYSIZE(feature_levels);

	UINT create_device_flags = 0;
	if (Global::GetDebug())
	{
		create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
	}

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		adapters[1].adapter_pointer, // adapter pointer
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL, // For softfare driver
		create_device_flags, // flags for runtime layers
		&feature_levels[1], // feature levels array
		num_feature_levels - 1, //  number of feature levels
		D3D11_SDK_VERSION,
		&scd, //swapchain description
		m_swap_chain_ptr.GetAddressOf(), // swapchain address
		m_device_ptr.GetAddressOf(), // device address
		NULL, // supported feature level
		m_device_context_ptr.GetAddressOf() // device context address
	);

	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create device and swapchain");
		return false;
	}

	Global::GetAnnotation().SetDeviceContext(m_device_context_ptr);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
	hr = m_swap_chain_ptr->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(back_buffer.GetAddressOf()));
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "GetBuffer failed");
		return false;
	}
	
	hr = m_device_ptr->CreateRenderTargetView(back_buffer.Get(), nullptr, m_render_taget_view_ptr.GetAddressOf());
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create render target view");
		return false;
	}

	// Create Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(width);
	viewport.Height = static_cast<FLOAT>(height);
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	m_device_context_ptr->RSSetViewports(1, &viewport);
	return true;
}

bool Graphics::initilize_shaders()
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA}
	};

	UINT num_elements = ARRAYSIZE(layout);

	if (!m_vertex_shader.Initialize(m_device_ptr, L"vertexshader.cso", layout, num_elements))
	{
		return false;
	}

	if (!m_brdf_pixel_shader.Initialize(m_device_ptr, L"brdfpixelshader.cso"))
	{
		return false;
	}

	if (!m_geometry_pixel_shader.Initialize(m_device_ptr, L"geometrypixelshader.cso"))
	{
		return false;
	}

	if (!m_ndf_pixel_shader.Initialize(m_device_ptr, L"ndfpixelshader.cso"))
	{
		return false;
	}

	if (!m_fresnel_pixel_shader.Initialize(m_device_ptr, L"fresnelpixelshader.cso"))
	{
		return false;
	}

	if (!m_env_pixel_shader.Initialize(m_device_ptr, L"envpixelshader.cso"))
	{
		return false;
	}

	return true;
}

bool Graphics::initilize_scene()
{
	using namespace DirectX;
	DirectX::XMFLOAT3 m_position{ 0.0f, 0.0f, 0.0f };
	static size_t constexpr SPHERE_PARTS = 100;
	static float constexpr RADIUS = 50.f;
	using namespace DirectX;
	float delta_tetha = DirectX::XM_PI / SPHERE_PARTS; //thetta
	float delta_phi = 2 * DirectX::XM_PI / SPHERE_PARTS;
	size_t layer = 0;
	size_t cur_layer_size = 0;
	size_t prev_layer_size = 0;
	float tetha = 0;
	for (; layer <= SPHERE_PARTS; tetha += delta_tetha, ++layer)
	{
		prev_layer_size = cur_layer_size;

		if (tetha > DirectX::XM_PI)
		{
			tetha = DirectX::XM_PI;
		}
		cur_layer_size = SPHERE_PARTS + 1;
		int ind = 0;
		for (float phi = 0; phi < 2 * DirectX::XM_PI; phi += delta_phi)
		{
			float n_x = sin(tetha) * sin(phi);
			float n_y = cos(tetha);
			float n_z = sin(tetha) * cos(phi);
			float x = RADIUS * n_x + m_position.x;
			float y = RADIUS * n_y + m_position.y;
			float z = RADIUS * n_z + m_position.z;
			m_sphere_vertex.push_back({ XMFLOAT4(x, y, z, 1.0f), XMFLOAT3(n_x, n_y, n_z), XMFLOAT2(static_cast<float>(ind) / (SPHERE_PARTS), static_cast<float>(layer) / (SPHERE_PARTS)) });
			++ind;
		}
		{
			float n_x = sin(tetha) * sin(0.0f);
			float n_y = cos(tetha);
			float n_z = sin(tetha) * cos(0.0f);
			float x = RADIUS * n_x + m_position.x;
			float y = RADIUS * n_y + m_position.y;
			float z = RADIUS * n_z + m_position.z;
			m_sphere_vertex.push_back({ XMFLOAT4(x, y, z, 1.0f), XMFLOAT3(n_x, n_y, n_z), XMFLOAT2(static_cast<float>(ind) / (SPHERE_PARTS), static_cast<float>(layer) / (SPHERE_PARTS)) });
		}
		if (layer > 0)
		{
			size_t cur_layer_start = m_sphere_vertex.size() - cur_layer_size;
			size_t prev_layer_start = cur_layer_start - prev_layer_size;
			for (size_t i = 0; i < SPHERE_PARTS; ++i)
			{
				m_sphere_indicies.push_back(cur_layer_start + i + 1);
				m_sphere_indicies.push_back(cur_layer_start + i);
				m_sphere_indicies.push_back(prev_layer_start + i);
				
				m_sphere_indicies.push_back(prev_layer_start + i + 1);
				m_sphere_indicies.push_back(cur_layer_start + i + 1);
				m_sphere_indicies.push_back(prev_layer_start + i);
			}

		}
	}

	D3D11_BUFFER_DESC vertex_buffer_desc;
	ZeroMemory(&vertex_buffer_desc, sizeof(vertex_buffer_desc));

	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = sizeof(Vertex) * m_sphere_vertex.size();
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertex_buffer_data;
	ZeroMemory(&vertex_buffer_data, sizeof(vertex_buffer_data));
	vertex_buffer_data.pSysMem = m_sphere_vertex.data();

	HRESULT hr = m_device_ptr->CreateBuffer(&vertex_buffer_desc, &vertex_buffer_data, m_vertex_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create vertex buffer");
		return false;
	}


	D3D11_BUFFER_DESC index_buffer_desc;
	ZeroMemory(&index_buffer_desc, sizeof(index_buffer_desc));
	index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	index_buffer_desc.ByteWidth = sizeof(WORD) * m_sphere_indicies.size();
	index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	index_buffer_desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA index_sub_data;
	ZeroMemory(&index_sub_data, sizeof(index_sub_data));
	index_sub_data.pSysMem = m_sphere_indicies.data();
	
	hr = m_device_ptr->CreateBuffer(&index_buffer_desc, &index_sub_data, m_index_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create index buffer");
		return false;
	}

	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));
	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	constant_buffer_desc.ByteWidth = sizeof(ConstantBuffer);
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constant_buffer_desc.CPUAccessFlags = 0;

	hr = m_device_ptr->CreateBuffer(&constant_buffer_desc, nullptr, m_constant_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create consatnt buffer.");
		return false;
	}

	m_world1 = DirectX::XMMatrixIdentity();
	m_view = DirectX::XMMatrixLookAtLH(m_camera.eye, m_camera.at, m_camera.up);
	m_projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, m_width / (FLOAT)m_height, 0.01f, 100.0f);

	hr = CreateDDSTextureFromFile(m_device_ptr.Get(), L"texture.dds", nullptr, m_texture_resource_view.GetAddressOf());
	if (FAILED(hr))
		return false;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_device_ptr->CreateSamplerState(&sampDesc, m_sampler_linear.GetAddressOf());
	if (FAILED(hr))
		return false;


	return true;
}

bool Graphics::initialize_lights()
{
	using namespace DirectX;
	// Setup our lighting parameters
	XMFLOAT4 vLightDirs[NUM_OF_LIGHT] =
	{
		XMFLOAT4(0.0f, 0.0f, -10.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, 10.0f, 1.0f),
		XMFLOAT4(5.0f, 5.0f, -5.0f, 1.0f),
	};
	XMFLOAT4 vLightColors[NUM_OF_LIGHT] =
	{
		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
		XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f)
	};
	float vLightIntencitys[NUM_OF_LIGHT] =
	{
		1.0f, 1.0f, 1.0f
	};
	for (int m = 0; m < NUM_OF_LIGHT; m++) {
		m_vLightDirs[m] = vLightDirs[m];
		m_vLightColors[m] = vLightColors[m];
		m_vLightIntencitys[m] = vLightIntencitys[m];
		m_vLightIntencityIndex[m] = 0;
	}

	return true;
}

bool Graphics::update_texture()
{
	return true;
}

bool Graphics::initialize_tone_maping()
{
	return true;
}

bool Graphics::create_depth_stencil_buffer(size_t width, size_t height)
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.ArraySize = 1;
	depthDesc.MipLevels = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.Height = height;
	depthDesc.Width = width;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;

	hr = m_device_ptr->CreateTexture2D(&depthDesc, NULL, &m_depth_ptr);
	if (SUCCEEDED(hr))
	{
		hr = m_device_ptr->CreateDepthStencilView(m_depth_ptr.Get(), NULL, &m_depthDSV_ptr);
	}

	return SUCCEEDED(hr);
}

bool Graphics::OnResizeWindow(size_t width, size_t height)
{
	if (m_swap_chain_ptr)
	{
		m_device_context_ptr->OMSetRenderTargets(0, 0, 0);
		m_render_taget_view_ptr->Release();
		m_device_context_ptr->Flush();
		m_width = width;
		m_height = height;
		m_projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, static_cast<FLOAT>(m_width) / static_cast<FLOAT>(m_height), 0.01f, 100.0f);
		HRESULT hr = m_swap_chain_ptr->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(hr))
		{
			utils::WinErrorLogger::Log(hr, "Falied to resize buffers");
			return false;
		}
		m_viewport.Width = static_cast<FLOAT>(width);
		m_viewport.Height = static_cast<FLOAT>(height);

		ID3D11Texture2D* back_buffer = nullptr;
		hr = m_swap_chain_ptr->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));
		if (FAILED(hr))
		{
			utils::WinErrorLogger::Log(hr, "Failed to create swap chain");
			return false;
		}
		hr = m_device_ptr->CreateRenderTargetView(back_buffer, nullptr, m_render_taget_view_ptr.GetAddressOf());
		if (FAILED(hr))
		{
			utils::WinErrorLogger::Log(hr, "Failed to create render target view");
			return false;
		}
		back_buffer->Release();
		if (!create_depth_stencil_buffer(width, height)) {
			return false;
		}
		return m_tone_maping.OnResizeWindow(width, height) && m_render_in_texture.Initialize(m_device_ptr, width, height);
	}
	else
	{
		return true;
	}
}