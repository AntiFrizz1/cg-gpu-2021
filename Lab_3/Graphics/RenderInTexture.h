#pragma once
#include <stdexcept>
#include <d3d11.h>
#include <wrl/client.h>

class RenderInTexture
{
public:
    RenderInTexture() : m_format(DXGI_FORMAT_R16G16B16A16_FLOAT), m_viewport() {}
    RenderInTexture(DXGI_FORMAT format) : m_format(format), m_viewport() {}
    RenderInTexture(DXGI_FORMAT format, Microsoft::WRL::ComPtr<ID3D11Device> device,
                    size_t width,
        size_t height)
        : m_format(format)
        , m_viewport() 
    {
        if (!Initialize(device, width, height))
        {
            throw new std::runtime_error("Failed to create texture render.");
        }
    }

    bool Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device,
                    size_t width,
                    size_t height);

    ID3D11RenderTargetView* GetTextureRenderTargetView() const { return m_texture_render_target_view.Get(); };
    ID3D11ShaderResourceView* GetTextureShaderResourceView() const { return m_texture_shader_resource_view.Get(); };
    ID3D11Texture2D* GetTextureRenderTarget() const { return m_texture_render_target.Get(); };
    D3D11_VIEWPORT GetViewPort() const { return m_viewport; }

private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_texture_render_target;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>   m_texture_render_target_view;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture_shader_resource_view;

    DXGI_FORMAT    m_format;
    D3D11_VIEWPORT m_viewport;
};