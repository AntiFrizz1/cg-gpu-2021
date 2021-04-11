#pragma once

#include <DirectXMath.h>

#define NUM_OF_LIGHT 3
__declspec(align(16))
struct ConstantBuffer
{
    DirectX::XMMATRIX world; // 16
    DirectX::XMMATRIX view; // 16
    DirectX::XMMATRIX projection;// 16
    DirectX::XMFLOAT4 vLightDir[NUM_OF_LIGHT]; //12
    DirectX::XMFLOAT4 vLightColor[NUM_OF_LIGHT]; // 12
    DirectX::XMFLOAT4 vLightIntencity[NUM_OF_LIGHT];
    DirectX::XMFLOAT4 eye;
    float             metalness;
    float             roughness;
    DirectX::XMFLOAT3 albedo{0.91, 0.92, 0.92};
};

__declspec(align(16))
struct LuminanceConstantBuffer
{
    float AverageLuminance;
};