#include "Camera.h"


DirectX::XMFLOAT3 constexpr EYE{ 0.0f, 0.0f, -10.0f };
DirectX::XMFLOAT3 constexpr AT{ 0.0f, 0.0f, 1.0f };
DirectX::XMFLOAT3 constexpr UP{ 0.0f, 1.0f, 0.0f };

	Camera::Camera()
		: m_eye(DirectX::XMVectorSet(EYE.x, EYE.y, EYE.z, 0.0f))
		, m_at(DirectX::XMVectorSet(AT.x, AT.y, AT.z, 0.0f))
		, m_up(DirectX::XMVectorSet(UP.x, UP.y, UP.z, 0.0f))
		, m_position(0, 0, EYE.z)
		, m_lat(0)
		, m_lon(0)
		, m_radius(EYE.z)
		, need_update(false)
	{
		UpdateCameraPosition();
		UpdateCameraView();
	}

	void Camera::SetLat(float value)
	{
		m_lat = value;
		need_update = true;
	}
	void Camera::SetLon(float value)
	{
		m_lon = value;
		need_update = true;
	}
	void Camera::SetRadius(float value)
	{
		m_radius = value;
		need_update = true;
	}

	void Camera::AddLat(float value)
	{
		m_lat += value;
		if (m_lat <= -(float)DirectX::XM_PI / 2)
		{
			m_lat = -(float)DirectX::XM_PI / 2;
		}
		if (m_lat >= (float)DirectX::XM_PI / 2)
		{
			m_lat = (float)DirectX::XM_PI / 2;
		}
		need_update = true;
	}
	void Camera::AddLon(float value)
	{

		m_lon += value;
		need_update = true;
	}
	void Camera::AddRadius(float value)
	{
		
		m_radius += value;
		if (m_radius < 0)
			m_radius = 0;
		need_update = true;
	}

	DirectX::XMFLOAT3 Camera::GetPosition()
	{
		if (need_update) 
		{
			UpdateCameraView();
			UpdateCameraPosition();
			need_update = false;
		}

		return m_position;
	}

	DirectX::XMMATRIX Camera::GetView()
	{
		if (need_update) 
		{
			UpdateCameraView();
			UpdateCameraPosition();
			need_update = false;
		}

		return m_view;
	}

	void Camera::UpdateCameraPosition()
	{
		float x, y, z;
		m_position.x = -m_radius * cos(m_lat) * sin(m_lon);
		m_position.y = - m_radius * sin(m_lon);
		m_position.z = -m_radius * cos(m_lon) * cos(m_lat);

	}

	void Camera::UpdateCameraView()
	{
		m_view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixTranslation(0, 0, -m_radius) * DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_lon));
	}


	/////////////////////
	DirectX::XMVECTOR Camera::GetForwardCameraDir() {
		return DirectX::XMVector3Transform(m_at, DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_lon));
	}

	DirectX::XMVECTOR Camera::GetBackwardCameraDir() {
		return DirectX::XMVector3Transform(m_at, DirectX::XMMatrixRotationAxis({ 0,1,0 }, DirectX::XM_PI) * DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_lon));
	}

	DirectX::XMVECTOR Camera::GetRightCameraDir() {
		return DirectX::XMVector3Transform(m_at, DirectX::XMMatrixRotationAxis({ 0,1,0 }, DirectX::XM_PI / 2.0) * DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_lon));

	}
	DirectX::XMVECTOR Camera::GetUpCameraDir() {
		return  DirectX::XMVector3Transform(m_at, DirectX::XMMatrixRotationAxis({ 1,0,0 }, DirectX::XM_PI / 2.0) * DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_lon));

	}


