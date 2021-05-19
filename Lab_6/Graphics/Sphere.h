#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

#include "Vertex.h"
#include "ConstantBuffer.h"

class Sphere
{
public:
	static size_t constexpr SPHERE_PARTS = 100;
	static float constexpr RADIUS = 2.f;

	Sphere() = default;
	Sphere(Sphere const& other);

	bool Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device);
	void MoveVertexes(DirectX::XMFLOAT3 direction);

	bool UpdateBuffers(Microsoft::WRL::ComPtr<ID3D11Device> device) { return create_buffers(device); }

	ID3D11Buffer* GetVertexBuffer() { return m_vertex_buffer.Get(); }
	ID3D11Buffer* GetIndexBuffer() { return m_index_buffer.Get(); }

	ID3D11Buffer** GetAddressOfVertexBuffer() { return m_vertex_buffer.GetAddressOf(); }
	ID3D11Buffer** GetAddressOfIndexBuffer() { return m_index_buffer.GetAddressOf(); }

	size_t GetIniciesSize() const { return m_sphere_indicies.size(); }

private:
	void create_vertex_and_indicies();
	bool create_buffers(Microsoft::WRL::ComPtr<ID3D11Device> device);

	std::vector<Vertex> m_sphere_vertex;
	std::vector<WORD> m_sphere_indicies;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_index_buffer;
	
	DirectX::XMFLOAT3 m_position{ 0.0f, 0.0f, 0.0f };
};