#pragma once

#include <DirectXMath.h>

DirectX::XMFLOAT3 constexpr EYE{ 0.0f, 0.0f, -15.0f };
DirectX::XMFLOAT3 constexpr AT{ 0.0f, 0.0f, 1.0f };
DirectX::XMFLOAT3 constexpr UP{ 0.0f, 1.0f, 0.0f };

struct CameraPosition
{
	CameraPosition()
		: eye(DirectX::XMVectorSet(EYE.x, EYE.y, EYE.z, 0.0f))
		, at(DirectX::XMVectorSet(AT.x, AT.y, AT.z, 0.0f))
		, up(DirectX::XMVectorSet(UP.x, UP.y, UP.z, 0.0f))
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

struct WorldCameraPosition
{
	float lat{ 0.0f };
	float lon{ 0.0f };
	float pos_x{ EYE.x };
	float pos_y{ EYE.y };
	float pos_z{ EYE.z };
};