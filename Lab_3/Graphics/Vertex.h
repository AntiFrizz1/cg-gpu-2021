#pragma once
#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>

struct Vertex
{
	DirectX::XMFLOAT4 pos;
	//DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 tex;
};
