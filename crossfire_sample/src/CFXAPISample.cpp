//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
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
#include "CFXAPISample.h"

#include <dxgi1_2.h>
#include <d3d11.h>
#include <iostream>
#include <d3dcompiler.h>
#include <algorithm>

#include <iostream>

#include "Window.h"

#pragma warning( disable : 4100 ) // disable unreference formal parameter warnings for /W4 builds

#ifdef max 
#undef max
#endif

using namespace Microsoft::WRL;

namespace AMD {
namespace {
struct Vertex
{
	float position[3];
	float uv[2];
};
}
///////////////////////////////////////////////////////////////////////////////
CFXAPISample::CFXAPISample()
{
}

///////////////////////////////////////////////////////////////////////////////
CFXAPISample::~CFXAPISample()
{
}

///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::PrepareRender()
{
	static const float clearColor [] = {
		0.042f, 0.042f, 0.042f,
		1
	};

	m_deviceContext->ClearRenderTargetView (m_renderTargetView, clearColor);
}

///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::Render()
{
	PrepareRender();

	static int frameNumber = 0;
	frameNumber++;

	m_deviceContext->OMSetDepthStencilState (m_depthStencilState, 0);
	m_deviceContext->VSSetShader (m_vertexShader, nullptr, 0);
	m_deviceContext->PSSetShader (m_pixelShader, nullptr, 0);
	m_deviceContext->IASetIndexBuffer (m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer* vertexBuffers[] = { m_vertexBuffer };
	UINT strides[] = { sizeof (Vertex) };
	UINT offsets[] = { 0 };
	m_deviceContext->IASetVertexBuffers (0, 1, vertexBuffers, strides, offsets);

	m_deviceContext->IASetInputLayout (m_inputLayout);
	m_deviceContext->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	/**
	What happens here is:

	* On every odd frame, render an increasingly red color into texture_
	* In every frame, blit texture into the back buffer
	*/

	if (m_cfxEnabled) {
		// We're starting to use texture_, notify the API. Notice that we need
		// to notify even if we don't actually write to it in this frame.
		agsDriverExtensionsDX11_NotifyResourceBeginAllAccess (m_agsContext, m_texture);
	}
	
	if ((frameNumber & 1) == 1) {
		const float clearColor[] = {
			(frameNumber % 256) / 256.0f, 0.042f, 0.042f,
			1
		};

		D3D11_VIEWPORT viewports[1];
		viewports[0].Height = 1080;
		viewports[0].Width = 1920;
		viewports[0].MinDepth = 0;
		viewports[0].MaxDepth = 1;
		viewports[0].TopLeftX = 0;
		viewports[0].TopLeftY = 0;
		m_deviceContext->RSSetViewports (1, viewports);

		ID3D11ShaderResourceView* psSRVs[] = {
			m_uploadTextureSRV
		};

		ID3D11RenderTargetView* renderTargetViews[] = {
			m_textureRTV
		};	
		m_deviceContext->OMSetRenderTargets (1, renderTargetViews, nullptr);

		m_deviceContext->PSSetShaderResources (0, 1, psSRVs);
		m_deviceContext->UpdateSubresource (m_uploadTexture, 0, nullptr, clearColor, sizeof (float) * 4, sizeof (float) * 4);
		m_deviceContext->DrawIndexed (6, 0, 0);

		if (m_cfxEnabled) {
			// We're done with writes to texture_, notify the API so it can
			// start copying
			agsDriverExtensionsDX11_NotifyResourceEndWrites (m_agsContext, m_texture, nullptr, nullptr, 0);
		}
	}

	D3D11_VIEWPORT viewports[1];
	viewports[0].Height = 1080;
	viewports[0].Width = 1920;
	viewports[0].MinDepth = 0;
	viewports[0].MaxDepth = 1;
	viewports[0].TopLeftX = 0;
	viewports[0].TopLeftY = 0;
	m_deviceContext->RSSetViewports (1, viewports);

	ID3D11RenderTargetView* renderTargetViews[] = {
		m_renderTargetView
	};

	m_deviceContext->OMSetRenderTargets (1, renderTargetViews, nullptr);
	ID3D11ShaderResourceView* psSRVs[] = {
		m_textureSRV
	};

	m_deviceContext->PSSetShaderResources (0, 1, psSRVs);
	m_deviceContext->DrawIndexed (6, 0, 0);

	if (m_cfxEnabled) {
		// We're done using texture_ for this frame, notify the API
		agsDriverExtensionsDX11_NotifyResourceEndAllAccess (m_agsContext, m_texture);
	}

	ID3D11ShaderResourceView* psNullSRVs[] = {
		nullptr
	};
	m_deviceContext->PSSetShaderResources (0, 1, psNullSRVs);
	
	FinalizeRender ();
}

///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::FinalizeRender ()
{
}

///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::Run (const int frameCount)
{
	Initialize ();

	for (int i = 0; i < frameCount; ++i) {	
		Render ();
		Present ();
	}

	Shutdown ();
}

///////////////////////////////////////////////////////////////////////////////
/**
Present the current frame by swapping the back buffer, then move to the
next back buffer and also signal the fence for the current queue slot entry.
*/
void CFXAPISample::Present ()
{
	m_swapChain->Present (1, 0);
}


///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::Initialize ()
{
	m_window.reset (new Window ("AMD CFX API test", 1920, 1080));

	// Call this before device creation
	agsInit( &m_agsContext, nullptr, &m_agsGPUInfo );

	CreateDeviceAndSwapChain();

	CreateMeshBuffers();

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = false;
	dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	m_device->CreateDepthStencilState (&dsDesc, &m_depthStencilState);

	ComPtr<ID3DBlob> vsCode, psCode;
	D3DCompileFromFile (L"..\\..\\src\\Shaders\\shaders.hlsl", nullptr, nullptr, "VS_main", "vs_5_0", 0, 0, &vsCode, nullptr);
	D3DCompileFromFile (L"..\\..\\src\\Shaders\\shaders.hlsl", nullptr, nullptr, "PS_main", "ps_5_0", 0, 0, &psCode, nullptr);

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	m_device->CreateVertexShader (vsCode->GetBufferPointer (), vsCode->GetBufferSize (), 0, &m_vertexShader);
	m_device->CreatePixelShader (psCode->GetBufferPointer (), psCode->GetBufferSize (), nullptr, &m_pixelShader);
	m_device->CreateInputLayout (layoutDesc, 2, vsCode->GetBufferPointer (), vsCode->GetBufferSize (), &m_inputLayout);

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.Height = 1;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Width = 1;

	if (m_cfxEnabled) {
		// Our texture is also a render target and is only updated every second
		// frame. Create it using the Crossfire API to enable transfers on it.
		agsDriverExtensionsDX11_CreateTexture2D (m_agsContext, &textureDesc, nullptr, &m_texture,
			// If set to TransferDisable, flickering will be present
			// as the updated version is not transfered to the second GPU.
			// With TransferApp1StepP2P, the Crossfire API will copy the
			// data to the second GPU.
			AGS_AFR_TRANSFER_1STEP_P2P, AGS_AFR_TRANSFERENGINE_DEFAULT);
	} else {
		m_device->CreateTexture2D (&textureDesc, nullptr, &m_texture);
	}
	
	m_device->CreateShaderResourceView (m_texture, nullptr, &m_textureSRV);
	m_device->CreateRenderTargetView (m_texture, nullptr, &m_textureRTV);

	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	m_device->CreateTexture2D (&textureDesc, nullptr, &m_uploadTexture);
	m_device->CreateShaderResourceView (m_uploadTexture, nullptr, &m_uploadTextureSRV);
}

///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::Shutdown ()
{
	// Clear things out to avoid false-positive D3D debug runtime ref-count warnings.
	m_swapChain->SetFullscreenState (FALSE, 0);
	m_deviceContext->ClearState ();
	m_deviceContext->Flush ();

    m_vertexBuffer->Release();
    m_indexBuffer->Release();

	m_renderTargetView->Release();
    m_renderTarget->Release();
	
	m_textureSRV->Release();
	m_textureRTV->Release();
    m_texture->Release();

    m_uploadTextureSRV->Release();
	m_uploadTexture->Release();

	m_depthStencilState->Release();
    m_inputLayout->Release();
	m_pixelShader->Release();
	m_vertexShader->Release();

    m_swapChain->Release();

    if ( m_agsGPUInfo.devices[ 0 ].vendorId == 0x1002 )
    {
        agsDriverExtensionsDX11_DestroyDevice( m_agsContext, m_device, nullptr, m_deviceContext, nullptr );
    }
    else
    {
        m_deviceContext->Release();
        m_device->Release();
    }

	agsDeInit(m_agsContext);
}

///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::CreateDeviceAndSwapChain ()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.Height = 1080;
	swapChainDesc.BufferDesc.Width = 1920;
	swapChainDesc.OutputWindow = m_window->GetHWND ();
	swapChainDesc.Windowed = true;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.SampleDesc.Count = 1;

    AGSDX11DeviceCreationParams creationParams = 
    {
        nullptr,                    /* default adapter */
        D3D_DRIVER_TYPE_HARDWARE,
        0,                          /* Module for driver, must be null if hardware*/
#ifdef _DEBUG
	    D3D11_CREATE_DEVICE_DEBUG,  /* debug device for debug builds */
#else
		0,                          /* No flags */
#endif       
        nullptr,                    /* no feature levels */
        0,                          /* no feature levels */
        D3D11_SDK_VERSION,
        &swapChainDesc
    };

    if ( m_agsGPUInfo.devices[ 0 ].vendorId == 0x1002 )
    {
        AGSDX11ExtensionParams extensionParams = {};
        extensionParams.crossfireMode = AGS_CROSSFIRE_MODE_EXPLICIT_AFR; // Enable AFR without requiring a driver profile
        extensionParams.uavSlot = 7;
        AGSDX11ReturnedParams returnedParams = {};

        if ( agsDriverExtensionsDX11_CreateDevice( m_agsContext, &creationParams, &extensionParams, &returnedParams ) == AGS_SUCCESS )
        {
            m_device = returnedParams.pDevice;
            m_deviceContext = returnedParams.pImmediateContext;
            m_swapChain = returnedParams.pSwapChain;

            if ( returnedParams.extensionsSupported & AGS_DX11_EXTENSION_CROSSFIRE_API )
            {
                m_cfxEnabled = true;
            }
        }
    }
    else
    {
        D3D11CreateDeviceAndSwapChain( 
            creationParams.pAdapter,
            creationParams.DriverType,
            creationParams.Software,
            creationParams.Flags,
            creationParams.pFeatureLevels,
            creationParams.FeatureLevels,
            creationParams.SDKVersion,
		    &swapChainDesc,
		    &m_swapChain,
		    &m_device,
		    nullptr,
		    &m_deviceContext);
    }

	m_swapChain->GetBuffer (0, IID_PPV_ARGS (&m_renderTarget));
	m_device->CreateRenderTargetView (m_renderTarget, nullptr, &m_renderTargetView);
}

///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::CreateMeshBuffers ()
{
	static const Vertex vertices [4] = {
		// Upper Left
		{ { -1.0f, 1.0f, 0 },{ 0, 0 } },
		// Upper Right
		{ { 1.0f, 1.0f, 0 },{ 1, 0 } },
		// Bottom right
		{ { 1.0f, -1.0f, 0 },{ 1, 1 } },
		// Bottom left
		{ { -1.0f, -1.0f, 0 },{ 0, 1 } }
	};

	static const int indices [6] = {
		0, 1, 2, 2, 3, 0
	};

	D3D11_BUFFER_DESC vbDesc;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.ByteWidth = sizeof (vertices);
	vbDesc.CPUAccessFlags = 0;
	vbDesc.MiscFlags = 0;
	vbDesc.StructureByteStride = sizeof (Vertex);
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA vbInitialData;
	vbInitialData.pSysMem = vertices;
	vbInitialData.SysMemPitch = sizeof (vertices);
	vbInitialData.SysMemSlicePitch = sizeof (vertices);

	m_device->CreateBuffer (&vbDesc, &vbInitialData, &m_vertexBuffer);

	D3D11_BUFFER_DESC ibDesc;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.ByteWidth = sizeof (indices);
	ibDesc.CPUAccessFlags = 0;
	ibDesc.MiscFlags = 0;
	ibDesc.StructureByteStride = sizeof (int);
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA ibInitialData;
	ibInitialData.pSysMem = indices;
	ibInitialData.SysMemPitch = sizeof (indices);
	ibInitialData.SysMemSlicePitch = sizeof (indices);

	m_device->CreateBuffer (&ibDesc, &ibInitialData, &m_indexBuffer);
}
}

int main (int argc, char* argv [])
{
	// Enable run-time memory check for debug builds.
	// (When _DEBUG is not defined, calls to _CrtSetDbgFlag are removed during preprocessing.)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	AMD::CFXAPISample sample;
	sample.Run (512);	
}