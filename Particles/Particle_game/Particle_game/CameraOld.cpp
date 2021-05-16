#include "CameraOld.h"

DirectX::XMFLOAT3 constexpr EYE{ 0.0f, 0.0f, -10.0f };
DirectX::XMFLOAT3 constexpr AT{ 0.0f, 0.0f, 1.0f };
DirectX::XMFLOAT3 constexpr UP{ 0.0f, 1.0f, 0.0f };

CameraOld::CameraOld()
	: m_eye(DirectX::XMVectorSet(EYE.x, EYE.y, EYE.z, 0.0f))
	, m_at(DirectX::XMVectorSet(AT.x, AT.y, AT.z, 0.0f))
	, m_up(DirectX::XMVectorSet(UP.x, UP.y, UP.z, 0.0f))
	, m_position(0, 0, EYE.z)
	, m_lat(0)
	, m_lon(0)
	, need_update(false)
{
	UpdateCameraView();
}

void CameraOld::SetLat(float value)
{
	m_lat = value;
	need_update = true;
}
void CameraOld::SetLon(float value)
{
	m_lon = value;
	need_update = true;
}

void CameraOld::AddLat(float value)
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
void CameraOld::AddLon(float value)
{
	m_lon += value;
	need_update = true;
}

void CameraOld::MoveForvard(float value) {

	DirectX::XMVECTOR direct = GetForwardCameraDir();
	m_position.x += DirectX::XMVectorGetX(direct) * value;
	m_position.y += DirectX::XMVectorGetY(direct) * value;
	m_position.z += DirectX::XMVectorGetZ(direct) * value;
	need_update = true;
}
void CameraOld::MoveRight(float value) {
	DirectX::XMVECTOR direct = GetRightCameraDir();
	m_position.x += DirectX::XMVectorGetX(direct) * value;
	m_position.y += DirectX::XMVectorGetY(direct) * value;
	m_position.z += DirectX::XMVectorGetZ(direct) * value;
	need_update = true;
}
void CameraOld::MoveUp(float value) {
	DirectX::XMVECTOR direct = GetUpCameraDir();
	m_position.x += DirectX::XMVectorGetX(direct) * value;
	m_position.y += DirectX::XMVectorGetY(direct) * value;
	m_position.z += DirectX::XMVectorGetZ(direct) * value;
	need_update = true;
}



DirectX::XMFLOAT3 CameraOld::GetPosition()
{
	UpdateCameraView();
	return m_position;
}

DirectX::XMMATRIX CameraOld::GetView()
{
	if (need_update) 
	{
		UpdateCameraView();
		need_update = false;
	}
	return m_view;
}


void CameraOld::UpdateCameraView()
{
	m_view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_lon) * DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z));
}

DirectX::XMVECTOR CameraOld::GetForwardCameraDir() {
	return DirectX::XMVector3Transform(m_at, DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_lon));
}

DirectX::XMVECTOR CameraOld::GetBackwardCameraDir() {
	return DirectX::XMVector3Transform(m_at, DirectX::XMMatrixRotationAxis({ 0,1,0 }, DirectX::XM_PI) * DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_lon));
}

DirectX::XMVECTOR CameraOld::GetRightCameraDir() {
	return DirectX::XMVector3Transform(m_at, DirectX::XMMatrixRotationAxis({ 0,1,0 }, DirectX::XM_PI / 2.0) * DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_lon));

}
DirectX::XMVECTOR CameraOld::GetUpCameraDir() {
	return  DirectX::XMVector3Transform(m_at, DirectX::XMMatrixRotationAxis({ 1,0,0 }, DirectX::XM_PI / 2.0) * DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_lon));

}

