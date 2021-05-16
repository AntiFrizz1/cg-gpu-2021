#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera();
	void SetLat(float value);
	void SetLon(float value);
	void SetRadius(float value);

	void AddLat(float value);
	void AddLon(float value);
	void AddRadius(float value);
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMMATRIX GetView();

	void UpdateCameraPosition();
	void UpdateCameraView();

/////////////////////
	DirectX::XMVECTOR GetForwardCameraDir();
	DirectX::XMVECTOR GetBackwardCameraDir();
	DirectX::XMVECTOR GetRightCameraDir();
	DirectX::XMVECTOR GetUpCameraDir();

private:
	DirectX::XMVECTOR m_eye;
	DirectX::XMVECTOR m_at;
	DirectX::XMVECTOR m_up;

	DirectX::XMMATRIX m_view;
	DirectX::XMFLOAT3 m_position;
	float m_lat;
	float m_lon;
	float m_radius;
	bool need_update;

};

