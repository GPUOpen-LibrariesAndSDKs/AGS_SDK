//
// Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "ExtensionsSample12.h"

#include "d3dx12.h"
#include <d3dcompiler.h>
#include <assert.h>
#include <DirectXMath.h>

using namespace Microsoft::WRL;
using namespace DirectX;

namespace AMD
{

struct ConstantBuffer
{
    XMFLOAT4X4 transform;
};


///////////////////////////////////////////////////////////////////////////////
void ExtensionsSample12::CreateTexture (ID3D12GraphicsCommandList * uploadCommandList)
{
    int width = 256, height = 256;

    DWORD *imageData = (DWORD*)malloc(width*height*sizeof(DWORD));
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int xx = x / 32;
            int yy = y / 32;

            imageData[x + y*width] = (xx % 2 == yy % 2) ? 0xffffffff : 0xff000000;
        }
    }

    static const auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_DEFAULT);
    const auto resourceDesc = CD3DX12_RESOURCE_DESC::Tex2D (DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, width, height, 1, 1);

    m_device->CreateCommittedResource (&defaultHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS (&m_image));

    static const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_UPLOAD);
    const auto uploadBufferSize = GetRequiredIntermediateSize (m_image.Get (), 0, 1);
    const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer (uploadBufferSize);

    m_device->CreateCommittedResource (&uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &uploadBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS (&m_uploadImage));

    D3D12_SUBRESOURCE_DATA srcData;
    srcData.pData = imageData;
    srcData.RowPitch = width * 4;
    srcData.SlicePitch = width * height * 4;

    UpdateSubresources (uploadCommandList, m_image.Get (), m_uploadImage.Get (), 0, 0, 1, &srcData);
    const auto transition = CD3DX12_RESOURCE_BARRIER::Transition (m_image.Get (),
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    uploadCommandList->ResourceBarrier (1, &transition);

    free(imageData);

    D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    shaderResourceViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

    m_device->CreateShaderResourceView (m_image.Get (), &shaderResourceViewDesc, m_srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart ());
}

///////////////////////////////////////////////////////////////////////////////
void ExtensionsSample12::RenderImpl (ID3D12GraphicsCommandList * commandList)
{
    static int counter = 0;
    counter++;

    if ( m_agsContext )
    {
        agsDriverExtensionsDX12_PushMarker( m_agsContext, commandList, "Render" );
    }

    commandList->SetGraphicsRootSignature( m_rootSignature.Get() );

    // Set our state (shaders, etc.)
    commandList->SetPipelineState(m_triPSO.Get ());

    // Set the descriptor heap containing the texture srv
    ID3D12DescriptorHeap* heaps[] = { m_srvDescriptorHeap.Get () };
    commandList->SetDescriptorHeaps (1, heaps);

    // Set slot 0 of our root signature to point to our descriptor heap with
    // the texture SRV
    commandList->SetGraphicsRootDescriptorTable(0, m_srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart ());

    if ( m_agsDeviceExtensions.intrinsics16 )
    {
        // Set a dummy descriptor for the UAV we use for the AGS extension mechanism - this stops the GPU-based validation layer from complaining.
        commandList->SetGraphicsRootDescriptorTable( 2, m_dummyUAVGPUHandle );
    }

    UpdateConstantBuffer( commandList, 0.0f, 0.0f, std::abs (sinf (static_cast<float> (counter) / 64.0f)) );

    commandList->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    commandList->IASetVertexBuffers( 0, 1, &m_triVertexBufferView );
    commandList->IASetIndexBuffer( &m_indexBufferView );
    commandList->DrawIndexedInstanced( 3, 1, 0, 0, 0 );

    if ( m_agsContext )
    {
        agsDriverExtensionsDX12_PopMarker( m_agsContext, commandList );
    }
}

///////////////////////////////////////////////////////////////////////////////
void ExtensionsSample12::InitializeImpl (ID3D12GraphicsCommandList * uploadCommandList)
{
    // We need one descriptor heap to store our texture SRV which cannot go
    // into the root signature. So create a SRV type heap with one entry
    D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
    descriptorHeapDesc.NumDescriptors = 2;
    // This heap contains SRV, UAV or CBVs -- in our case one SRV
    descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    descriptorHeapDesc.NodeMask = 0;
    descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    m_device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS (&m_srvDescriptorHeap));

    CreateRootSignature();
    CreatePipelineStateObject();
    CreateMeshBuffers(uploadCommandList);
    CreateTexture(uploadCommandList);
}

///////////////////////////////////////////////////////////////////////////////
void ExtensionsSample12::ShutdownImpl()
{
}

///////////////////////////////////////////////////////////////////////////////
void ExtensionsSample12::CreateMeshBuffers (ID3D12GraphicsCommandList* uploadCommandList)
{
    struct TriVertex
    {
        float position[3];
        float uv[2];
    };

    float aspectRatio = (float)m_viewport.Width / (float)m_viewport.Height;

    static const TriVertex triVertices[] =
    {
        { {  0.0f,   0.25f * aspectRatio, 0.0f }, { 0.5f, 0.0f } },
        { {  0.25f, -0.25f * aspectRatio, 0.0f }, { 1.0f, 1.0f } },
        { { -0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 1.0f } }
    };

    static const int indices[] =
    {
        0, 1, 2
    };

    static const int uploadBufferSize = sizeof(triVertices) + sizeof(indices);
    static const auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_UPLOAD);
    static const auto uploadBufferDesc = CD3DX12_RESOURCE_DESC::Buffer (uploadBufferSize);

    // Create upload buffer on CPU
    m_device->CreateCommittedResource (&uploadHeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &uploadBufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS (&m_uploadBuffer));

    // Create vertex & index buffer on the GPU
    // HEAP_TYPE_DEFAULT is on GPU, we also initialize with COPY_DEST state
    // so we don't have to transition into this before copying into them
    static const auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_DEFAULT);

    CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer (sizeof (triVertices));
    m_device->CreateCommittedResource (&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS (&m_triVertexBuffer));

    const CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer (sizeof (indices));
    m_device->CreateCommittedResource (&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &indexBufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS (&m_indexBuffer));

    // Create buffer views
    m_triVertexBufferView.BufferLocation = m_triVertexBuffer->GetGPUVirtualAddress ();
    m_triVertexBufferView.SizeInBytes = sizeof (triVertices);
    m_triVertexBufferView.StrideInBytes = sizeof (TriVertex);

    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress ();
    m_indexBufferView.SizeInBytes = sizeof (indices);
    m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;

    // Copy data on CPU into the upload buffer
    void* p;
    m_uploadBuffer->Map (0, nullptr, &p);
    ::memcpy (p, triVertices, sizeof (triVertices));
    ::memcpy (static_cast<unsigned char*>(p) + sizeof(triVertices), indices, sizeof(indices));
    m_uploadBuffer->Unmap (0, nullptr);

    // Copy data from upload buffer on CPU into the index/vertex buffer on the GPU
    uploadCommandList->CopyBufferRegion( m_triVertexBuffer.Get (), 0, m_uploadBuffer.Get (), 0, sizeof (triVertices) );
    uploadCommandList->CopyBufferRegion( m_indexBuffer.Get (), 0, m_uploadBuffer.Get (), sizeof (triVertices), sizeof (indices) );

    // Barriers, batch them together
    const CD3DX12_RESOURCE_BARRIER barriers[] =
    {
        CD3DX12_RESOURCE_BARRIER::Transition( m_triVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER ),
        CD3DX12_RESOURCE_BARRIER::Transition( m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER )
    };

    uploadCommandList->ResourceBarrier( _countof( barriers ), barriers );
}


///////////////////////////////////////////////////////////////////////////////
void ExtensionsSample12::UpdateConstantBuffer( ID3D12GraphicsCommandList* commandList, float x, float y, float scale )
{
    XMMATRIX transform = XMMatrixTranslation( x, y, 0.0f ) * XMMatrixScaling( scale, scale, 1.0f );

    ConstantBuffer cb = {};
    XMStoreFloat4x4( &cb.transform, transform );
    commandList->SetGraphicsRoot32BitConstants( 1, sizeof(cb) / 4, &cb, 0 );
}

///////////////////////////////////////////////////////////////////////////////
void ExtensionsSample12::CreateRootSignature ()
{
    // We have two root parameters, one is a pointer to a descriptor heap
    // with a SRV, the second is a constant buffer view
    CD3DX12_ROOT_PARAMETER parameters[3] = {};

    // Create a descriptor table with one entry in our descriptor heap
    CD3DX12_DESCRIPTOR_RANGE range[2] = {};
    range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
    parameters[0].InitAsDescriptorTable (1, range);

    // Our constant buffer
    parameters[1].InitAsConstants( sizeof( ConstantBuffer ) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX );

    // We don't use another descriptor heap for the sampler, instead we use a
    // static sampler
    CD3DX12_STATIC_SAMPLER_DESC samplers[1] = {};
    samplers[0].Init(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature = {};

    // Create the root signature
    if ( m_agsDeviceExtensions.intrinsics16 )
    {
        //*** add AMD Intrinsic Resource ***
        range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, AGS_DX12_SHADER_INTRINSICS_SPACE_ID); // u0
        parameters[2].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_ALL);
        descRootSignature.Init(3, parameters, 1, samplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    }
    else
    {
        descRootSignature.Init(2, parameters, 1, samplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    }

    ComPtr<ID3DBlob> rootBlob;
    ComPtr<ID3DBlob> errorBlob;
    D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);

    m_device->CreateRootSignature(0, rootBlob->GetBufferPointer (), rootBlob->GetBufferSize (), IID_PPV_ARGS (&m_rootSignature));

    //
    // Create the dummy UAV that we have to bind 
    //
    auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_DEFAULT);
    auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(4);
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    m_device->CreateCommittedResource (&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS (&m_dummyUAV));


    D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptor = m_srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_dummyUAVGPUHandle = m_srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
    CPUDescriptor.ptr += m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
    m_dummyUAVGPUHandle.ptr += m_device->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
    desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    desc.Format = DXGI_FORMAT_R32_UINT;
    desc.Buffer.NumElements = 1;
    m_device->CreateUnorderedAccessView( m_dummyUAV.Get(), nullptr, &desc, CPUDescriptor );
}

///////////////////////////////////////////////////////////////////////////////
void ExtensionsSample12::CreatePipelineStateObject ()
{
    const D3D12_INPUT_ELEMENT_DESC triLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    const D3D12_INPUT_ELEMENT_DESC rectLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    //*********Cannot use D3DCOMPILE_SKIP_OPTIMIZATION flag with AMD Intrinsic extension!****************
    compileFlags &= ~D3DCOMPILE_SKIP_OPTIMIZATION;

    static const D3D_SHADER_MACRO useAGSMacros[] =
    {
        { "AMD_USE_SHADER_INTRINSICS", "1" },
        { nullptr, nullptr }
    };

    const D3D_SHADER_MACRO* macros = m_agsDeviceExtensions.intrinsics16 ? useAGSMacros : nullptr;

    ID3DBlob* pErrorMsgs;
    HRESULT hr;
    
    //*********Remember you need to use a 5_1 shader model***************
    ComPtr<ID3DBlob> triVertexShader;
    hr = D3DCompileFromFile(L"..\\src\\shaders.hlsl", macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_tri", "vs_5_1", compileFlags, 0, &triVertexShader, &pErrorMsgs);
    if (pErrorMsgs != NULL)
    {
        MessageBoxA(NULL, (char *)pErrorMsgs->GetBufferPointer(), "Error compiling VS", 0);
        pErrorMsgs->Release();
    }

    ComPtr<ID3DBlob> triPixelShader;
    hr = D3DCompileFromFile(L"..\\src\\shaders.hlsl", macros, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_tri", "ps_5_1", compileFlags, 0, &triPixelShader, &pErrorMsgs);
    if (pErrorMsgs != NULL)
    {
        MessageBoxA(NULL, (char *)pErrorMsgs->GetBufferPointer(), "Error compiling PS", 0);
        pErrorMsgs->Release();
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.VS.BytecodeLength = triVertexShader->GetBufferSize();
    psoDesc.VS.pShaderBytecode = triVertexShader->GetBufferPointer();
    psoDesc.PS.BytecodeLength = triPixelShader->GetBufferSize();
    psoDesc.PS.pShaderBytecode = triPixelShader->GetBufferPointer();
    psoDesc.InputLayout.NumElements = _countof(triLayout);
    psoDesc.InputLayout.pInputElementDescs = triLayout;
    psoDesc.pRootSignature = m_rootSignature.Get ();
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC (D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC (D3D12_DEFAULT);
    psoDesc.SampleDesc.Count = 1;
    psoDesc.DepthStencilState.DepthEnable = false;
    psoDesc.DepthStencilState.StencilEnable = false;
    psoDesc.SampleMask = 0xFFFFFFFF;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    m_device->CreateGraphicsPipelineState (&psoDesc, IID_PPV_ARGS (&m_triPSO));
}

}
