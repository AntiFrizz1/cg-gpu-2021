#include "Sphere.h"
#include "../WinErrorLoger.h"

Sphere::Sphere(Sphere const& other)
{
}

bool Sphere::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device)
{
	create_vertex_and_indicies();
	return create_buffers(device);
}

void Sphere::MoveVertexes(DirectX::XMFLOAT3 direction)
{
	for (auto& [position, _, tmp] : m_sphere_vertex)
	{
		position.x += direction.x;
		position.y += direction.y;
		position.z += direction.z;
	}
}

void Sphere::create_vertex_and_indicies()
{
	using namespace DirectX;
	float delta_tetha = DirectX::XM_PI / SPHERE_PARTS; //thetta
	float delta_phi = 2 * DirectX::XM_PI / SPHERE_PARTS;
	size_t layer = 0;
	size_t cur_layer_size = 0;
	size_t prev_layer_size = 0;
	for (float tetha = 0; tetha < DirectX::XM_PI; tetha += delta_tetha)
	{
		prev_layer_size = cur_layer_size;
		if (layer == 0 || layer == SPHERE_PARTS - 1)
		{
			float n_x = sin(tetha) * sin(0);
			float n_y = cos(tetha);
			float n_z = sin(tetha) * cos(0);
			float x = RADIUS * n_x + m_position.x;
			float y = RADIUS * n_y + m_position.y;
			float z = RADIUS * n_z + m_position.z;
			m_sphere_vertex.push_back({ XMFLOAT4(x, y, z, 1.0f), XMFLOAT3(n_x, n_y, n_z) });
			cur_layer_size = 1;
		}
		else
		{
			cur_layer_size = SPHERE_PARTS;
			for (float phi = 0; phi < 2 * DirectX::XM_PI; phi += delta_phi)
			{
				float n_x = sin(tetha) * sin(phi);
				float n_y = cos(tetha);
				float n_z = sin(tetha) * cos(phi);
				float x = RADIUS * n_x + m_position.x;
				float y = RADIUS * n_y + m_position.y;
				float z = RADIUS * n_z + m_position.z;
				m_sphere_vertex.push_back({ XMFLOAT4(x, y, z, 1.0f), XMFLOAT3(n_x, n_y, n_z) });
			}
		}
		if (layer > 0)
		{
			size_t cur_layer_start = m_sphere_vertex.size() - cur_layer_size;
			size_t prev_layer_start = cur_layer_start - prev_layer_size;
			for (size_t i = 0; i < SPHERE_PARTS; ++i)
			{
				m_sphere_indicies.push_back(prev_layer_start + i % prev_layer_size);
				m_sphere_indicies.push_back(cur_layer_start + i % cur_layer_size);
				m_sphere_indicies.push_back(cur_layer_start + (i + 1) % cur_layer_size);
				m_sphere_indicies.push_back(prev_layer_start + i % prev_layer_size);
				m_sphere_indicies.push_back(cur_layer_start + (i + 1) % cur_layer_size);
				m_sphere_indicies.push_back(prev_layer_start + (i + 1) % prev_layer_size);
			}

		}
		++layer;
	}
}

bool Sphere::create_buffers(Microsoft::WRL::ComPtr<ID3D11Device> device)
{
	D3D11_BUFFER_DESC vertex_buffer_desc;
	ZeroMemory(&vertex_buffer_desc, sizeof(vertex_buffer_desc));

	vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	vertex_buffer_desc.ByteWidth = sizeof(Vertex) * m_sphere_vertex.size();
	vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_buffer_desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertex_buffer_data;
	ZeroMemory(&vertex_buffer_data, sizeof(vertex_buffer_data));
	vertex_buffer_data.pSysMem = m_sphere_vertex.data();
	
	if (m_vertex_buffer.Get()) 
	{ 
		m_vertex_buffer->Release(); 
		m_vertex_buffer.Reset();
	}
	HRESULT hr = device->CreateBuffer(&vertex_buffer_desc, &vertex_buffer_data, m_vertex_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create sphere vertex buffer");
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
	if (m_index_buffer.Get())
	{
		m_index_buffer->Release();
		m_index_buffer.Reset();
	}
	hr = device->CreateBuffer(&index_buffer_desc, &index_sub_data, m_index_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		utils::WinErrorLogger::Log(hr, "Failed to create sphere index buffer");
		return false;
	}

	return true;
}
