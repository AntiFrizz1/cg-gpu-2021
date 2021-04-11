#pragma once
#include <wrl/client.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <memory>

class Annotation
{
private:
    Annotation(Annotation&&) = delete;
    Annotation(Annotation const&) = delete;
    
    void release_annotation();
public:
    Annotation() = default;
    ~Annotation();

    bool SetDeviceContext(Microsoft::WRL::ComPtr<ID3D11DeviceContext> device_context);
    void BeginEvent(LPCWSTR name);
    void EndEvent();
private:
    ID3DUserDefinedAnnotation* m_instance{ nullptr };
};

class Global
{
private:
    Global() = default;
    Global(Global&&) = delete;
    Global(Global const&) = delete;

public:
    static Global& GetInstance();
    static Annotation& GetAnnotation();
    static bool GetDebug();
    Annotation annotation;
    bool debug_mode{ false };
};
