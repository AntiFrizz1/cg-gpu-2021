//#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "Model.h"
//#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#undef TINYGLTF_IMPLEMENTATION

DirectX::XMMATRIX create_matrix_from_node(tinygltf::Node& gltfNode);
D3D11_TEXTURE_ADDRESS_MODE get_texture_address_mode(int wrap);
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, D3D_SHADER_MACRO* pDefines); //////////////

Model::Model(const char* path): m_scale(1.0f), m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f)
{
	m_model_path = path;
}


Model::~Model() {

}

void Model::SetPixelShaderPath(const wchar_t* path) {
	m_pixel_shader_path = path;
}
void Model::SetVertexShaderPath(const wchar_t* path) {
	m_vertex_shader_path = path;
}

void Model::SetScale(float scale) {
    m_scale = scale;
}

void Model::SetPosition(float x, float y, float z) {
    m_position = DirectX::XMFLOAT3(x, y, z);
}

void Model::SetRotation(float x, float y, float z) {
    m_rotation = DirectX::XMFLOAT3(x, y, z);
}


bool Model::Load(ID3D11Device* device) {

    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&m_model, &err, &warn, m_model_path);
    //bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse glTF\n");
        return false;
    }
    // -------------------------------------------------------

    if(!create_shaders(device))
        return false;

    m_pixel_shader_ptrs.resize(16);
    m_shader_resource_view_ptrs.resize(m_model.images.size());

    if (!create_sampler_state(device))
        return false;

    if (!create_materials(device))
        return false;

    if (!create_primitives(device))
        return false;

	return true;
}


bool Model::create_sampler_state(ID3D11Device* device) {
	tinygltf::Model& model = m_model;

	HRESULT hr = S_OK;
    tinygltf::Sampler& gltfSampler = model.samplers[0];
    D3D11_SAMPLER_DESC sd;
    ZeroMemory(&sd, sizeof(sd));

    switch (gltfSampler.minFilter)
    {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
        if (gltfSampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
            sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        else
            sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        break;
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
        if (gltfSampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
            sd.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        else
            sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        break;
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
        if (gltfSampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
            sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        else
            sd.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
        break;
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
        if (gltfSampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
            sd.Filter = D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
        else
            sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        break;
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
        if (gltfSampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
            sd.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
        else
            sd.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
        break;
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
        if (gltfSampler.magFilter == TINYGLTF_TEXTURE_FILTER_NEAREST)
            sd.Filter = D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
        else
            sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        break;
    default:
        sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        break;
    }

    sd.AddressU = get_texture_address_mode(gltfSampler.wrapS);
    sd.AddressV = get_texture_address_mode(gltfSampler.wrapT);
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd.MinLOD = 0;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device->CreateSamplerState(&sd, m_sampler_state_ptr.GetAddressOf());

    return true;
}

D3D11_TEXTURE_ADDRESS_MODE get_texture_address_mode(int wrap)
{
    switch (wrap)
    {
    case TINYGLTF_TEXTURE_WRAP_REPEAT:
        return D3D11_TEXTURE_ADDRESS_WRAP;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
        return D3D11_TEXTURE_ADDRESS_MIRROR;
    default:
        return D3D11_TEXTURE_ADDRESS_WRAP;
    }
}

bool Model::create_texture(ID3D11Device* device, size_t imageIdx, bool useSRGB)
{
    HRESULT hr = S_OK;

    if (m_shader_resource_view_ptrs[imageIdx])
        return false;

    tinygltf::Image& gltfImage = m_model.images[imageIdx];

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    DXGI_FORMAT format = useSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
    CD3D11_TEXTURE2D_DESC td(format, gltfImage.width, gltfImage.height, 1, 1, D3D11_BIND_SHADER_RESOURCE);
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = gltfImage.image.data();
    initData.SysMemPitch = 4 * gltfImage.width;
    hr = device->CreateTexture2D(&td, &initData, &texture);
    if (FAILED(hr))
        return false;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
    CD3D11_SHADER_RESOURCE_VIEW_DESC srvd(D3D11_SRV_DIMENSION_TEXTURE2D, td.Format);
    hr = device->CreateShaderResourceView(texture.Get(), &srvd, &shader_resource_view);
    if (FAILED(hr))
        return false;
    m_shader_resource_view_ptrs[imageIdx] = shader_resource_view;

    return true;
}


bool Model::create_materials(ID3D11Device* device)
{
    HRESULT hr = S_OK;
    tinygltf::Model& model = m_model;

    for (tinygltf::Material& gltfMaterial : model.materials)
    {
        Material material = {};
        material.name = gltfMaterial.name;
        material.blend = false;

        D3D11_BLEND_DESC bd = {};
        // All materials have alpha mode "BLEND" or "OPAQUE"
        if (gltfMaterial.alphaMode == "BLEND")
        {
            material.blend = true;
            bd.RenderTarget[0].BlendEnable = true;
            bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
            bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
            bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            hr = device->CreateBlendState(&bd, &material.blend_state_ptr);
            if (FAILED(hr))
                return false;
        }

        D3D11_RASTERIZER_DESC rd = {};
        rd.FillMode = D3D11_FILL_SOLID;
        if (gltfMaterial.doubleSided)
            rd.CullMode = D3D11_CULL_NONE;
        else
            rd.CullMode = D3D11_CULL_BACK;
        rd.FrontCounterClockwise = true;
        rd.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
        rd.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
        rd.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        hr = device->CreateRasterizerState(&rd, &material.rasterizer_state_ptr);
        if (FAILED(hr))
            return false;

        float albedo[4];
        for (UINT i = 0; i < 4; ++i)
            albedo[i] = static_cast<float>(gltfMaterial.pbrMetallicRoughness.baseColorFactor[i]);
        material.material_buffer_data.albedo = DirectX::XMFLOAT4(albedo);
        material.material_buffer_data.metalness = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
        material.material_buffer_data.roughness = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);

        material.ps_defines_flags = 0;

        material.base_color_texture = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
        if (material.base_color_texture >= 0)
        {
            material.ps_defines_flags |= MATERIAL_HAS_COLOR_TEXTURE;
            if (!create_texture(device, material.base_color_texture, true))
                return false;
        }

        material.metallic_roughness_texture = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index;
        if (material.metallic_roughness_texture >= 0)
        {
            material.ps_defines_flags |= MATERIAL_HAS_METAL_ROUGH_TEXTURE;
            if (!create_texture(device, material.metallic_roughness_texture))
                return false;
        }

        material.normal_texture = gltfMaterial.normalTexture.index;
        if (material.normal_texture >= 0)
        {
            material.ps_defines_flags |= MATERIAL_HAS_NORMAL_TEXTURE;
            if (!create_texture(device, material.normal_texture))
                return false;
        }

        if (gltfMaterial.occlusionTexture.index >= 0)
            material.ps_defines_flags |= MATERIAL_HAS_OCCLUSION_TEXTURE;

        if (!create_pixel_shader(device, material.ps_defines_flags))
            return false;

        material.emissive_texture = gltfMaterial.emissiveTexture.index;
        if (material.emissive_texture >= 0)
        {
            if (!create_texture(device, material.emissive_texture, true))
                return false;
        }

        m_materials.push_back(material);
    }

    return true;
}

bool Model::create_primitives(ID3D11Device* device)
{
    tinygltf::Scene& gltfScene = m_model.scenes[m_model.defaultScene];

    m_world_matricies.push_back(DirectX::XMMatrixIdentity());

    for (int node : gltfScene.nodes)
    {
        if (!process_node(device, node, m_world_matricies[0]))
            return false;
    }

    return true;
}

bool Model::process_node(ID3D11Device* device, int node, DirectX::XMMATRIX world_matrix)
{
    tinygltf::Node& gltfNode = m_model.nodes[node];

    if (gltfNode.mesh >= 0)
    {
        tinygltf::Mesh& gltfMesh = m_model.meshes[gltfNode.mesh];
        UINT matrixIndex = static_cast<UINT>(m_world_matricies.size() - 1);

        for (tinygltf::Primitive& gltfPrimitive : gltfMesh.primitives)
        {
            if (!create_primitive(device, gltfPrimitive, matrixIndex))
                return false;
        }
    }

    if (gltfNode.children.size() > 0)
    {
        world_matrix = DirectX::XMMatrixMultiplyTranspose(DirectX::XMMatrixTranspose(world_matrix), DirectX::XMMatrixTranspose(create_matrix_from_node(gltfNode)));
        m_world_matricies.push_back(world_matrix);
        for (int childNode : gltfNode.children)
        {
            if (!process_node(device, childNode, world_matrix))
                return false;
        }
    }

    return true;
}


DirectX::XMMATRIX create_matrix_from_node(tinygltf::Node& gltfNode)
{
    if (gltfNode.matrix.empty())
    {
        DirectX::XMMATRIX rotation;
        if (gltfNode.rotation.empty())
            rotation = DirectX::XMMatrixIdentity();
        else
        {
            float v[4] = {};
            float* p = v;
            for (double value : gltfNode.rotation)
            {
                *p = static_cast<float>(value);
                ++p;
            }
            DirectX::XMFLOAT4 vector(v);
            rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&vector));
        }
        return rotation;
    }
    else
    {
        float flat[16] = {};
        float* p = flat;
        for (double value : gltfNode.matrix)
        {
            *p = static_cast<float>(value);
            ++p;
        }
        DirectX::XMFLOAT4X4 matrix(flat);
        return DirectX::XMLoadFloat4x4(&matrix);
    }
}




bool Model::create_primitive(ID3D11Device* device, tinygltf::Primitive& gltfPrimitive, UINT matrix)
{
    HRESULT hr = S_OK;
    tinygltf::Model& model = m_model;
    Primitive primitive = {};
    primitive.matrix = matrix;

    for (std::pair<const std::string, int>& item : gltfPrimitive.attributes)
    {
        if (item.first == "TEXCOORD_1") // It isn't used in model
            continue;

        tinygltf::Accessor& gltfAccessor = model.accessors[item.second];
        tinygltf::BufferView& gltfBufferView = model.bufferViews[gltfAccessor.bufferView];
        tinygltf::Buffer& gltfBuffer = model.buffers[gltfBufferView.buffer];

        Attribute attribute = {};
        attribute.byteStride = static_cast<UINT>(gltfAccessor.ByteStride(gltfBufferView));

        CD3D11_BUFFER_DESC vbd(attribute.byteStride * static_cast<UINT>(gltfAccessor.count), D3D11_BIND_VERTEX_BUFFER);
        vbd.StructureByteStride = attribute.byteStride;
        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
        initData.pSysMem = &gltfBuffer.data[gltfBufferView.byteOffset + gltfAccessor.byteOffset];
        hr = device->CreateBuffer(&vbd, &initData, &attribute.vertex_buffer_ptr);
        if (FAILED(hr))
            return false;

        primitive.attributes.push_back(attribute);

        if (item.first == "POSITION")
        {
            primitive.vertex_count = static_cast<UINT>(gltfAccessor.count);

            DirectX::XMFLOAT3 maxPosition(static_cast<float>(gltfAccessor.maxValues[0]), static_cast<float>(gltfAccessor.maxValues[1]), static_cast<float>(gltfAccessor.maxValues[2]));
            DirectX::XMFLOAT3 minPosition(static_cast<float>(gltfAccessor.minValues[0]), static_cast<float>(gltfAccessor.minValues[1]), static_cast<float>(gltfAccessor.minValues[2]));

            primitive.max = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&maxPosition), m_world_matricies[primitive.matrix]);
            primitive.min = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&minPosition), m_world_matricies[primitive.matrix]);
        }
    }

    switch (gltfPrimitive.mode)
    {
    case TINYGLTF_MODE_POINTS:
        primitive.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        break;
    case TINYGLTF_MODE_LINE:
        primitive.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        break;
    case TINYGLTF_MODE_LINE_STRIP:
        primitive.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        break;
    case TINYGLTF_MODE_TRIANGLES:
        primitive.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        break;
    case TINYGLTF_MODE_TRIANGLE_STRIP:
        primitive.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        break;
    }

    tinygltf::Accessor& gltfAccessor = model.accessors[gltfPrimitive.indices];
    tinygltf::BufferView& gltfBufferView = model.bufferViews[gltfAccessor.bufferView];
    tinygltf::Buffer& gltfBuffer = model.buffers[gltfBufferView.buffer];

    primitive.index_count = static_cast<uint32_t>(gltfAccessor.count);
    UINT stride = 2;
    switch (gltfAccessor.componentType)
    {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        primitive.indexFormat = DXGI_FORMAT_R8_UINT;
        stride = 1;
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        primitive.indexFormat = DXGI_FORMAT_R16_UINT;
        stride = 2;
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        primitive.indexFormat = DXGI_FORMAT_R32_UINT;
        stride = 4;
        break;
    }

    CD3D11_BUFFER_DESC ibd(stride * primitive.index_count, D3D11_BIND_INDEX_BUFFER);
    D3D11_SUBRESOURCE_DATA initData;
    ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
    initData.pSysMem = &gltfBuffer.data[gltfBufferView.byteOffset + gltfAccessor.byteOffset];
    hr = device->CreateBuffer(&ibd, &initData, &primitive.index_buffer_ptr);
    if (FAILED(hr))
        return false;

    primitive.material = gltfPrimitive.material;
    if (m_materials[primitive.material].blend)
    {
        m_transparent_primitives.push_back(primitive);
        if (m_materials[primitive.material].emissive_texture >= 0)
            m_emissive_transparent_primitives.push_back(primitive);
    }
    else
    {
        m_primitives.push_back(primitive);
        if (m_materials[primitive.material].emissive_texture >= 0)
            m_emissive_primitives.push_back(primitive);
    }

    return true;
}



bool Model::create_pixel_shader(ID3D11Device* device, UINT definesFlags)
{
    HRESULT hr = S_OK;

    if (m_pixel_shader_ptrs[definesFlags])
        return true;

    std::vector<D3D_SHADER_MACRO> defines;
    defines.push_back({ "HAS_TANGENT", "1" });

    if (definesFlags & MATERIAL_HAS_COLOR_TEXTURE)
        defines.push_back({ "HAS_COLOR_TEXTURE", "1" });

    if (definesFlags & MATERIAL_HAS_METAL_ROUGH_TEXTURE)
        defines.push_back({ "HAS_METAL_ROUGH_TEXTURE", "1" });;

    if (definesFlags & MATERIAL_HAS_NORMAL_TEXTURE)
        defines.push_back({ "HAS_NORMAL_TEXTURE", "1" });

    if (definesFlags & MATERIAL_HAS_OCCLUSION_TEXTURE)
        defines.push_back({ "HAS_OCCLUSION_TEXTURE", "1" });

    defines.push_back({ nullptr, nullptr });



    Microsoft::WRL::ComPtr<ID3DBlob> blob;
    hr = CompileShaderFromFile(m_pixel_shader_path.c_str(), "main", "ps_5_0", &blob, defines.data());
    if (FAILED(hr))
        return false;

    hr = device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_pixel_shader_ptrs[definesFlags]);

    return true;
}
//////////////////////////////////////////////////////////
HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut, D3D_SHADER_MACRO* pDefines)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> err;
    hr = D3DCompileFromFile(szFileName, pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &err);
    if (FAILED(hr) && err)
        OutputDebugStringA(reinterpret_cast<const char*>(err->GetBufferPointer()));

    return hr;
}
///////////////////////////////////////////////////////////////////


bool Model::create_shaders(ID3D11Device* device)
{
    HRESULT hr = S_OK;

    Microsoft::WRL::ComPtr<ID3DBlob> blob;

    std::vector<D3D_SHADER_MACRO> defines;
    defines.push_back({ "HAS_TANGENT", "1" });
    defines.push_back({ nullptr, nullptr });

    hr = CompileShaderFromFile(m_vertex_shader_path.c_str(), "main", "vs_5_0", &blob, defines.data());
    if (FAILED(hr))
        return false;

    hr = device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_pVertexShader);
    if (FAILED(hr))
        return false;

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD_", 0, DXGI_FORMAT_R32G32_FLOAT, 3, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), blob->GetBufferPointer(), blob->GetBufferSize(), &m_pInputLayout);
    if (FAILED(hr))
        return false;

    defines.resize(1);
    defines.push_back({ "HAS_EMISSIVE", "1" });
    defines.push_back({ nullptr, nullptr });

    hr = CompileShaderFromFile(m_pixel_shader_path.c_str(), "main", "ps_5_0", &blob, defines.data());
    if (FAILED(hr))
        return false;

    hr = device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_pEmissivePixelShader);

    return true;
}


void Model::render_primitive(Primitive& primitive, ID3D11DeviceContext* context, ConstantBuffer& transformationData, ID3D11Buffer* transformationConstantBuffer, ID3D11Buffer* materialConstantBuffer, ShadersSlots& slots, bool emissive)
{
    std::vector<ID3D11Buffer*> combined;
    std::vector<UINT> offset;
    std::vector<UINT> stride;

    size_t attributesCount = primitive.attributes.size();
    combined.resize(attributesCount);
    offset.resize(attributesCount);
    stride.resize(attributesCount);
    for (size_t i = 0; i < attributesCount; ++i)
    {
        combined[i] = primitive.attributes[i].vertex_buffer_ptr.Get();
        stride[i] = primitive.attributes[i].byteStride;
    }
    context->IASetVertexBuffers(0, static_cast<UINT>(attributesCount), combined.data(), stride.data(), offset.data());

    context->IASetIndexBuffer(primitive.index_buffer_ptr.Get(), primitive.indexFormat, 0);
    context->IASetPrimitiveTopology(primitive.primitive_topology);

    Material& material = m_materials[primitive.material];
    if (material.blend)
        context->OMSetBlendState(material.blend_state_ptr.Get(), nullptr, 0xFFFFFFFF);
    context->RSSetState(material.rasterizer_state_ptr.Get());

    context->IASetInputLayout(m_pInputLayout.Get());
    context->VSSetShader(m_pVertexShader.Get(), nullptr, 0);

    transformationData.world = DirectX::XMMatrixTranspose(m_world_matricies[primitive.matrix] * m_world_transformation);

    context->UpdateSubresource(transformationConstantBuffer, 0, NULL, &transformationData, 0, 0);

    MaterialConstantBuffer materialBufferData = material.material_buffer_data;
    if (emissive)
    {
        materialBufferData.albedo = DirectX::XMFLOAT4(1, 1, 1, materialBufferData.albedo.w);
        context->PSSetShaderResources(slots.base_color_texture_slot, 1, m_shader_resource_view_ptrs[material.emissive_texture].GetAddressOf());
        context->PSSetShader(m_pEmissivePixelShader.Get(), nullptr, 0);
    }
    else
    {
        context->PSSetShader(m_pixel_shader_ptrs[material.ps_defines_flags].Get(), nullptr, 0);
        if (material.base_color_texture >= 0)
            context->PSSetShaderResources(slots.base_color_texture_slot, 1, m_shader_resource_view_ptrs[material.base_color_texture].GetAddressOf());
        if (material.metallic_roughness_texture >= 0)
            context->PSSetShaderResources(slots.metallic_roughness_texture_slot, 1, m_shader_resource_view_ptrs[material.metallic_roughness_texture].GetAddressOf());
        if (material.normal_texture >= 0)
            context->PSSetShaderResources(slots.normal_texture_slot, 1, m_shader_resource_view_ptrs[material.normal_texture].GetAddressOf());
    }
    context->UpdateSubresource(materialConstantBuffer, 0, NULL, &material.material_buffer_data, 0, 0);

    context->DrawIndexed(primitive.index_count, 0, 0);

    if (material.blend)
        context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}


void Model::Render(ID3D11DeviceContext* context, ConstantBuffer transformation_cb_data, ID3D11Buffer* transformation_cb, ID3D11Buffer* material_cb, ShadersSlots slots, DirectX::XMFLOAT3 camera_forward_dir)
{
    m_world_transformation =  DirectX::XMMatrixRotationAxis({ 1,0,0 }, m_rotation.x);
    m_world_transformation *= DirectX::XMMatrixRotationAxis({ 0,1,0 }, m_rotation.y);
    m_world_transformation *= DirectX::XMMatrixRotationAxis({ 0,0,1 }, m_rotation.z);
    m_world_transformation *= DirectX::XMMatrixScaling(m_scale, m_scale, m_scale);
    m_world_transformation *= DirectX::XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
   

    context->VSSetConstantBuffers(slots.transformation_cb_slot, 1, &transformation_cb);
    context->PSSetConstantBuffers(slots.transformation_cb_slot, 1, &transformation_cb);
    context->PSSetConstantBuffers(slots.material_cb_slot, 1, &material_cb);
    context->PSSetSamplers(slots.sampler_state_slot, 1, m_sampler_state_ptr.GetAddressOf());

    transformation_cb_data.world = DirectX::XMMatrixIdentity();
    for (Primitive& primitive : m_primitives)
        render_primitive(primitive, context, transformation_cb_data, transformation_cb, material_cb, slots);

    sort_and_render_transparent(m_transparent_primitives, context, transformation_cb_data, transformation_cb, material_cb, slots, camera_forward_dir, false);

    for (Primitive& primitive : m_emissive_primitives)
        render_primitive(primitive, context, transformation_cb_data, transformation_cb, material_cb, slots);

    sort_and_render_transparent(m_emissive_transparent_primitives, context, transformation_cb_data, transformation_cb, material_cb, slots, camera_forward_dir, true);
    
}

bool CompareDistancePairs(const std::pair<float, size_t>& p1, const std::pair<float, size_t>& p2)
{
    return p1.first < p2.first;
}

void Model::sort_and_render_transparent(std::vector<Primitive> transparent_primitives, ID3D11DeviceContext* context, ConstantBuffer transformation_cb_data, ID3D11Buffer* transformation_cb, ID3D11Buffer* material_cb, ShadersSlots slots, DirectX::XMFLOAT3 camera_forward_dir_input, bool emissive)
{
    DirectX::XMVECTOR camera_forward_dir = DirectX::XMLoadFloat3(&camera_forward_dir_input);
    std::vector<std::pair<float, size_t>> distances;
    float distance;
    DirectX::XMVECTOR center;
    DirectX::XMVECTOR cameraPos = DirectX::XMLoadFloat4(&transformation_cb_data.eye);
    for (size_t i = 0; i < transparent_primitives.size(); ++i)
    {
        center = DirectX::XMVectorDivide(DirectX::XMVectorAdd(transparent_primitives[i].max, transparent_primitives[i].min), DirectX::XMVectorReplicate(2));
        distance = DirectX::XMVectorGetX(DirectX::XMVector3Dot(DirectX::XMVectorSubtract(center, cameraPos), camera_forward_dir));
        distances.push_back(std::pair<float, size_t>(distance, i));
    }

    std::sort(distances.begin(), distances.end(), CompareDistancePairs);

    for (auto iter = distances.rbegin(); iter != distances.rend(); ++iter)
        render_primitive(transparent_primitives[(*iter).second], context, transformation_cb_data, transformation_cb, material_cb, slots, emissive);
}