#pragma once
#pragma once
#include <DirectXMath.h>

class CameraOld
{
public:
	CameraOld();
	void SetLat(float value);
	void SetLon(float value);

	void AddLat(float value);
	void AddLon(float value);
	void MoveForvard(float value);
	void MoveRight(float value);
	void MoveUp(float value);
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMMATRIX GetView();

	void UpdateCameraView();

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
	bool need_update;

};

class Camera_old
{
};

