#pragma once

#include <vector>

#include "RenderInTexture.h"
#include "Shaders.h"

class AverageLuminanceCalculator
{
public:
    AverageLuminanceCalculator() 
    {
        QueryPerformanceFrequency(&m_frequency);
        QueryPerformanceCounter(&m_last_time);
    }

    bool Initialize(Microsoft::WRL::ComPtr<ID3D11Device> device_ptr, size_t width, size_t height);
    bool OnResizeWindow(size_t width, size_t height);
    ID3D11ShaderResourceView* GetResultShaderResourceView() const { return m_textures.back().GetTextureShaderResourceView(); }

    float process(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, 
                  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sourceTexture);
private:
    void render_texture(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context,
                        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> sourceTexture, 
                        RenderInTexture& dst, 
                        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader);

    bool create_textures(size_t width, size_t height);

    std::vector<RenderInTexture> m_textures;

    VertexShader m_vertex_shader;
    PixelShader  m_pixel_shader;
    PixelShader  m_luminance_pixel_shader;

    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampler_state;
    Microsoft::WRL::ComPtr<ID3D11Texture2D>    m_luminance_texture;
    Microsoft::WRL::ComPtr<ID3D11Device>       m_device_ptr;

    float m_adapted_luminance{ 0.0f };
    LARGE_INTEGER m_frequency;
    LARGE_INTEGER m_last_time;
};