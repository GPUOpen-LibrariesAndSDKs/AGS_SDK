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

#pragma once

#include "D3D12Sample.h"

//
// This SDK sample shows how to use shader intrinsic instructions on GCN GPU's in DX12.
// Just for clarity the code required to enable the intrinsic instructions is ifdef'ed.
//
// When enabled the pixel shader will use the barycentric instruction,
// when disabled it will just render a triangle using a checker board texture.
//

#include <vector>

namespace AMD
{
class ExtensionsSample12 : public D3D12Sample
{
private:

    void CreateTexture(ID3D12GraphicsCommandList* uploadCommandList);
    void CreateMeshBuffers(ID3D12GraphicsCommandList* uploadCommandList);
    void UpdateConstantBuffer( ID3D12GraphicsCommandList* commandList, float x, float y, float scale );
    void CreateRootSignature();
    void CreatePipelineStateObject();
    void RenderImpl(ID3D12GraphicsCommandList* commandList) override;
    void InitializeImpl(ID3D12GraphicsCommandList* uploadCommandList) override;
    void ShutdownImpl() override;

    Microsoft::WRL::ComPtr<ID3D12Resource>          m_uploadBuffer;

    Microsoft::WRL::ComPtr<ID3D12Resource>          m_triVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW                        m_triVertexBufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource>          m_indexBuffer;
    D3D12_INDEX_BUFFER_VIEW                         m_indexBufferView;

    Microsoft::WRL::ComPtr<ID3D12Resource>          m_image;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_uploadImage;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_srvDescriptorHeap;

    Microsoft::WRL::ComPtr<ID3D12Resource>          m_dummyUAV;
    D3D12_GPU_DESCRIPTOR_HANDLE                     m_dummyUAVGPUHandle;

    Microsoft::WRL::ComPtr<ID3D12PipelineState>     m_triPSO;
};
}   // namespace AMD

