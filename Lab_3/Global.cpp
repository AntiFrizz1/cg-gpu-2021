#include "WinErrorLoger.h"
#include "Global.h"

bool Annotation::SetDeviceContext(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context)
{
    HRESULT hr = device_context->QueryInterface(__uuidof(m_instance), reinterpret_cast<void**>(&m_instance));
    if (FAILED(hr))
    {
        utils::WinErrorLogger::Log(hr, "Failed to create annotation");
        return false;
    }
    return true;
}

Annotation::~Annotation()
{
    release_annotation();
}

void Annotation::release_annotation()
{
    if (m_instance)
    {
        m_instance->Release();
    }
}

void Annotation::BeginEvent(LPCWSTR name)
{
    if (m_instance)
    {
        m_instance->BeginEvent(name);
    }
}

void Annotation::EndEvent()
{
    if (m_instance)
    {
        m_instance->EndEvent();
    }
}

Global& Global::GetInstance()
{
    static Global instance;
    
    return instance;
}

Annotation& Global::GetAnnotation()
{
    return GetInstance().annotation;
}

bool Global::GetDebug()
{
    return GetInstance().debug_mode;
}
