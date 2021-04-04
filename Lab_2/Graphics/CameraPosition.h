#pragma once

#include <DirectXMath.h>

struct CameraPosition
{
	CameraPosition() 
		: eye(DirectX::XMVectorSet(0.0f, 0.0f, -15.0f, 0.0f))
		, at(DirectX::XMVectorSet(.0f, 0.0f, 1.0, 0.0f))
		, up(DirectX::XMVectorSet(0.0f, 1.0f, 0.0, 0.0f))
	{
	}

	CameraPosition(DirectX::XMVECTOR eye, DirectX::XMVECTOR at, DirectX::XMVECTOR up)
		: eye(eye)
		, at(at)
		, up(up)
	{
	}


	DirectX::XMVECTOR eye;
	DirectX::XMVECTOR at;
	DirectX::XMVECTOR up;
};