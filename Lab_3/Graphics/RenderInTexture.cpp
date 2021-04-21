#include "RenderInTexture.h"
#include "../WinErrorLoger.h"

bool RenderInTexture::Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr, size_t width, size_t height)
{
    // CD3D11_TEXTURE2D_DESC rtd;
    ZeroMemory(&m_rtd, sizeof(m_rtd));
    m_rtd.Format = m_format;
    m_rtd.Width = static_cast<UINT>(width);
    m_rtd.Height = static_cast<UINT>(height);
    m_rtd.ArraySize = 1;
    m_rtd.MipLevels = 1;
    m_rtd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    m_rtd.Usage = D3D11_USAGE_DEFAULT;
    m_rtd.CPUAccessFlags = 0;
    m_rtd.SampleDesc.Count = 1;
    m_rtd.SampleDesc.Quality = 0;
    m_rtd.MiscFlags = 0;

    HRESULT hr = device_ptr->CreateTexture2D(&m_rtd, nullptr, m_texture_render_target.ReleaseAndGetAddressOf());
    if (FAILED(hr))
    {
        utils::WinErrorLogger::Log(hr, "Failed to create render target texture.");
        return false;
    }

    CD3D11_RENDER_TARGET_VIEW_DESC rtvd;
    ZeroMemory(&rtvd, sizeof(rtvd));
    rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvd.Format = m_rtd.Format;

    hr = device_ptr->CreateRenderTargetView(m_texture_render_target.Get(), &rtvd, m_texture_render_target_view.ReleaseAndGetAddressOf());
    if (FAILED(hr))
    {
        utils::WinErrorLogger::Log(hr, "Failed to create render target view texture.");
        return false;
    }

    CD3D11_SHADER_RESOURCE_VIEW_DESC srvd(D3D11_SRV_DIMENSION_TEXTURE2D, m_rtd.Format, 0, 1);
    hr = device_ptr->CreateShaderResourceView(m_texture_render_target.Get(), &srvd, m_texture_shader_resource_view.ReleaseAndGetAddressOf());
    if (FAILED(hr))
    {
        utils::WinErrorLogger::Log(hr, "Failed to create shader resource view texture.");
        return false;
    }

    ZeroMemory(&m_viewport, sizeof(m_viewport));
    m_viewport.Width = static_cast<FLOAT>(width);
    m_viewport.Height = static_cast<FLOAT>(height);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0;
    m_viewport.TopLeftY = 0;
    return true;
}