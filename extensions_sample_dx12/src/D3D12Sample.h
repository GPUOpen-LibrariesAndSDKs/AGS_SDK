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

#include <d3d12.h>
#include <dxgi.h>
#include <wrl.h>
#include <memory>
#include "amd_ags.h"

namespace AMD
{
class Window;

///////////////////////////////////////////////////////////////////////////////
class D3D12Sample
{
public:
    D3D12Sample (const D3D12Sample&) = delete;
    D3D12Sample& operator= (const D3D12Sample&) = delete;

    D3D12Sample ();
    virtual ~D3D12Sample ();

    void Run ();

protected:
    int GetQueueSlot () const
    {
        return m_currentBackBuffer;
    }

    static const int QUEUE_SLOT_COUNT = 3;

    static int GetQueueSlotCount ()
    {
        return QUEUE_SLOT_COUNT;
    }

    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_rectScissor;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> m_device;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets [QUEUE_SLOT_COUNT];
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;

    HANDLE m_frameFenceEvents [QUEUE_SLOT_COUNT];
    Microsoft::WRL::ComPtr<ID3D12Fence> m_frameFences [QUEUE_SLOT_COUNT];
    UINT64 m_currentFenceValue;
    UINT64 m_fenceValues[QUEUE_SLOT_COUNT];

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_renderTargetDescriptorHeap;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

    AGSContext*                                 m_agsContext = nullptr;
    AGSGPUInfo                                  m_agsGPUInfo = {};
    AGSDX12ReturnedParams::ExtensionsSupported  m_agsDeviceExtensions = {};

    virtual void InitializeImpl(ID3D12GraphicsCommandList* uploadCommandList) = 0;
    virtual void ShutdownImpl() = 0;
    virtual void RenderImpl(ID3D12GraphicsCommandList* commandList) = 0;

private:
    void Initialize ();
    void Shutdown ();

    void PrepareRender ();
    void FinalizeRender ();

    void Render ();
    void Present ();

    void CreateDeviceAndSwapChain ();
    void CreateAllocatorsAndCommandLists ();
    void CreateViewportScissor ();
    void SetupSwapChain ();
    void SetupRenderTargets ();

    std::unique_ptr<Window> m_window = {};

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocators[QUEUE_SLOT_COUNT] = {};
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandLists[QUEUE_SLOT_COUNT] = {};

    int m_currentBackBuffer = 0;

    unsigned int m_renderTargetViewDescriptorSize = 0;
};
}   // namespace AMD
