#include "AverageLuminanceCalculator.h"
#include <cmath>
#include "../Global.h"

bool AverageLuminanceCalculator::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr, size_t width, size_t height)
{
    if (!m_vertex_shader.Initialize(device_ptr, L"color_vertex_shader.cso", nullptr, 0))
    {
        return false;
    }


    if (!m_pixel_shader.Initialize(device_ptr, L"color_pixel_shader.cso"))
    {
        return false;
    }

    if (!m_luminance_pixel_shader.Initialize(device_ptr, L"luminance_pixel_shader.cso"))
    {
        return false;
    }
    m_device_ptr = device_ptr;
    // Create the sampler state
    D3D11_SAMPLER_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.MinLOD = 0;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    sd.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
    HRESULT hr = device_ptr->CreateSamplerState(&sd, m_sampler_state.GetAddressOf());
    if (FAILED(hr))
    {
        utils::WinErrorLogger::Log(hr, "Failed to create sampler state");
        return false;
    }

    CD3D11_TEXTURE2D_DESC ltd(
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        1,
        1,
        1,
        1,
        0,
        D3D11_USAGE_STAGING,
        D3D11_CPU_ACCESS_READ
    );
    hr = device_ptr->CreateTexture2D(&ltd, nullptr, &m_luminance_texture);
    if (FAILED(hr))
    {
        utils::WinErrorLogger::Log(hr, "Failed to create 2d texture");
        return false;
    }
    return create_textures(width, height);
}

bool AverageLuminanceCalculator::create_textures(size_t width, size_t height)
{
    size_t min_side_size = static_cast<size_t>(min(width, height));
    size_t num_of_textures = static_cast<size_t>(std::log2(min_side_size));
    m_textures.clear();

    try
    {
        for (size_t i = 0; i <= num_of_textures; i++)
        {
            size_t texture_size = static_cast<size_t>(1) << (num_of_textures - i);
            m_textures.emplace_back(DXGI_FORMAT_R32G32B32A32_FLOAT, m_device_ptr, texture_size, texture_size);
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool AverageLuminanceCalculator::OnResizeWindow(size_t width, size_t height)
{
    return create_textures(width, height);
}

void AverageLuminanceCalculator::render_texture(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
                                              Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sourceTexture,
                                              RenderInTexture& dst,
                                              Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader)
{
    ID3D11RenderTargetView* renderTarget = dst.GetTextureRenderTargetView();

    D3D11_VIEWPORT viewport = dst.GetViewPort();

    context->OMSetRenderTargets(1, &renderTarget, nullptr);
    context->RSSetViewports(1, &viewport);

    context->PSSetShader(pixelShader.Get(), nullptr, 0);
    context->PSSetShaderResources(0, 1, sourceTexture.GetAddressOf());

    context->Draw(4, 0);

    ID3D11ShaderResourceView* nullsrv[] = { nullptr };
    context->PSSetShaderResources(0, 1, nullsrv);
}

float AverageLuminanceCalculator::process(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
                                         Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sourceTexture)
{
    Global::GetAnnotation().BeginEvent(L"Start Average luminance processing.");
    float bgc[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    for (size_t i = 0; i < m_textures.size(); i++)
    {
        context->ClearRenderTargetView(m_textures[i].GetTextureRenderTargetView(), bgc);
    }

    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    context->VSSetShader(m_vertex_shader.GetShaderPtr(), nullptr, 0);
    context->PSSetSamplers(0, 1, m_sampler_state.GetAddressOf());

    render_texture(context, sourceTexture, m_textures[0], m_pixel_shader.GetShaderPtr());

    for (size_t i = 1; i < m_textures.size(); i++)
    {
        render_texture(context, m_textures[i - 1].GetTextureShaderResourceView(), m_textures[i], m_luminance_pixel_shader.GetShaderPtr());
    }

    ID3D11ShaderResourceView* nullsrv[] = { nullptr };
    context->PSSetShaderResources(0, 1, nullsrv);
    
    LARGE_INTEGER cur_time;
    QueryPerformanceCounter(&cur_time);
    size_t time_delta = static_cast<size_t>(cur_time.QuadPart - m_last_time.QuadPart);
    m_last_time = cur_time;
    double delta = static_cast<double>(time_delta) / m_frequency.QuadPart;


    D3D11_MAPPED_SUBRESOURCE luminance_accessor;
    context->CopyResource(m_luminance_texture.Get(), m_textures[m_textures.size() - 1].GetTextureRenderTarget());
    context->Map(m_luminance_texture.Get(), 0, D3D11_MAP_READ, 0, &luminance_accessor);

    float luminance = *(float*)luminance_accessor.pData;
    context->Unmap(m_luminance_texture.Get(), 0);

    float sigma = 0.04f / (0.04f + luminance);
    float tau = sigma * 0.4f + (1 - sigma) * 0.1f;
    m_adapted_luminance += (luminance - m_adapted_luminance) * static_cast<float>(1 - std::exp(-delta * tau));
    Global::GetAnnotation().EndEvent();
    return m_adapted_luminance;
}