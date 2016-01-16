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
CFXAPISample::CFXAPISample ()
: cfxEnabled_ (false)
{
}

///////////////////////////////////////////////////////////////////////////////
CFXAPISample::~CFXAPISample ()
{
}

///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::PrepareRender ()
{
	static const float clearColor [] = {
		0.042f, 0.042f, 0.042f,
		1
	};

	deviceContext_->ClearRenderTargetView (renderTargetView_.Get (), clearColor);
}

///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::Render ()
{
	PrepareRender ();

	static int frameNumber = 0;
	frameNumber++;

	deviceContext_->OMSetDepthStencilState (depthStencilState_.Get (), 0);
	deviceContext_->VSSetShader (vertexShader_.Get (), nullptr, 0);
	deviceContext_->PSSetShader (pixelShader_.Get (), nullptr, 0);
	deviceContext_->IASetIndexBuffer (indexBuffer_.Get (), DXGI_FORMAT_R32_UINT, 0);

	ID3D11Buffer* vertexBuffers[] = { vertexBuffer_.Get () };
	UINT strides[] = { sizeof (Vertex) };
	UINT offsets[] = { 0 };
	deviceContext_->IASetVertexBuffers (0, 1, vertexBuffers, strides, offsets);

	deviceContext_->IASetInputLayout (inputLayout_.Get ());
	deviceContext_->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	/**
	What happens here is:

	* On every odd frame, render an increasingly red color into texture_
	* In every frame, blit texture into the back buffer
	*/

	if (cfxEnabled_) {
		// We're starting to use texture_, notify the API. Notice that we need
		// to notify even if we don't actually write to it in this frame.
		agsDriverExtensions_NotifyResourceBeginAllAccess (agsContext_, texture_.Get ());
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
		deviceContext_->RSSetViewports (1, viewports);

		ID3D11ShaderResourceView* psSRVs[] = {
			uploadTextureSRV_.Get ()
		};

		ID3D11RenderTargetView* renderTargetViews[] = {
			textureRTV_.Get ()
		};	
		deviceContext_->OMSetRenderTargets (1, renderTargetViews, nullptr);

		deviceContext_->PSSetShaderResources (0, 1, psSRVs);
		deviceContext_->UpdateSubresource (uploadTexture_.Get (),
			0, nullptr, clearColor, sizeof (float) * 4, sizeof (float) * 4);
		deviceContext_->DrawIndexed (6, 0, 0);

		if (cfxEnabled_) {
			// We're done with writes to texture_, notify the API so it can
			// start copying
			agsDriverExtensions_NotifyResourceEndWrites (agsContext_, texture_.Get (),
				nullptr, nullptr, 0);
		}
	}

	D3D11_VIEWPORT viewports[1];
	viewports[0].Height = 1080;
	viewports[0].Width = 1920;
	viewports[0].MinDepth = 0;
	viewports[0].MaxDepth = 1;
	viewports[0].TopLeftX = 0;
	viewports[0].TopLeftY = 0;
	deviceContext_->RSSetViewports (1, viewports);

	ID3D11RenderTargetView* renderTargetViews[] = {
		renderTargetView_.Get ()
	};

	deviceContext_->OMSetRenderTargets (1, renderTargetViews, nullptr);
	ID3D11ShaderResourceView* psSRVs[] = {
		textureSRV_.Get ()
	};

	deviceContext_->PSSetShaderResources (0, 1, psSRVs);
	deviceContext_->DrawIndexed (6, 0, 0);

	if (cfxEnabled_) {
		// We're done using texture_ for this frame, notify the API
		agsDriverExtensions_NotifyResourceEndAllAccess (agsContext_, texture_.Get ());
	}

	ID3D11ShaderResourceView* psNullSRVs[] = {
		nullptr
	};
	deviceContext_->PSSetShaderResources (0, 1, psNullSRVs);
	
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
	swapChain_->Present (1, 0);
}

void CFXAPISample::InitializeAMDCFXAPI ()
{
	agsInit (&agsContext_, nullptr);

	unsigned int supportedExtensions = 0;
	agsDriverExtensions_Init (agsContext_, device_.Get (), &supportedExtensions);

	if ((supportedExtensions & AGS_EXTENSION_CROSSFIRE_API) == AGS_EXTENSION_CROSSFIRE_API) {
		agsDriverExtensions_SetCrossfireMode (agsContext_, AGS_CROSSFIRE_MODE_EXPLICIT_AFR);

		cfxEnabled_ = true;
	} else {
		cfxEnabled_ = false;
	}
}

///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::Initialize ()
{
	window_.reset (new Window ("AMD CFX API test", 1920, 1080));

	CreateDeviceAndSwapChain ();

	InitializeAMDCFXAPI ();

	CreateMeshBuffers ();

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = false;
	dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

	device_->CreateDepthStencilState (&dsDesc, &depthStencilState_);

	ComPtr<ID3DBlob> vsCode, psCode;
	D3DCompileFromFile (L"..\\src\\Shaders\\shaders.hlsl", nullptr, nullptr, "VS_main", "vs_5_0", 0, 0, &vsCode, nullptr);
	D3DCompileFromFile (L"..\\src\\Shaders\\shaders.hlsl", nullptr, nullptr, "PS_main", "ps_5_0", 0, 0, &psCode, nullptr);

	D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	device_->CreateVertexShader (vsCode->GetBufferPointer (), vsCode->GetBufferSize (), 0, &vertexShader_);
	device_->CreatePixelShader (psCode->GetBufferPointer (), psCode->GetBufferSize (), nullptr, &pixelShader_);
	device_->CreateInputLayout (layoutDesc, 2, vsCode->GetBufferPointer (), vsCode->GetBufferSize (), &inputLayout_);

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

	if (cfxEnabled_) {
		// Our texture is also a render target and is only updated every second
		// frame. Create it using the Crossfire API to enable transfers on it.
		agsDriverExtensions_CreateTexture2D (agsContext_, &textureDesc, nullptr, &texture_,
			// If set to TransferDisable, flickering will be present
			// as the updated version is not transfered to the second GPU.
			// With TransferApp1StepP2P, the Crossfire API will copy the
			// data to the second GPU.
			AGS_AFR_TRANSFER_1STEP_P2P);
	} else {
		device_->CreateTexture2D (&textureDesc, nullptr, &texture_);
	}
	
	device_->CreateShaderResourceView (texture_.Get (), nullptr, &textureSRV_);
	device_->CreateRenderTargetView (texture_.Get (), nullptr, &textureRTV_);

	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	device_->CreateTexture2D (&textureDesc, nullptr, &uploadTexture_);
	device_->CreateShaderResourceView (uploadTexture_.Get (), nullptr, &uploadTextureSRV_);
}

///////////////////////////////////////////////////////////////////////////////
void CFXAPISample::Shutdown ()
{
	agsDriverExtensions_DeInit (agsContext_);
	agsDeInit (agsContext_);

	// Clear things out to avoid false-positive D3D debug runtime ref-count warnings.
	swapChain_->SetFullscreenState (FALSE, 0);
	deviceContext_->ClearState ();
	deviceContext_->Flush ();
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
	swapChainDesc.OutputWindow = window_->GetHWND ();
	swapChainDesc.Windowed = false;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.SampleDesc.Count = 1;

	D3D11CreateDeviceAndSwapChain (nullptr /* default adapter */,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL, /* Module for driver, must be null if hardware*/
#ifdef _DEBUG
		D3D11_CREATE_DEVICE_DEBUG, /* debug device for debug builds */
#else
		0, /* No flags */
#endif
		nullptr, /* no feature levels */
		0, /* no feature levels */
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&swapChain_,
		&device_,
		nullptr,
		&deviceContext_);

	swapChain_->GetBuffer (0, IID_PPV_ARGS (&renderTarget_));
	device_->CreateRenderTargetView (renderTarget_.Get (), nullptr, &renderTargetView_);
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

	device_->CreateBuffer (&vbDesc, &vbInitialData, &vertexBuffer_);

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

	device_->CreateBuffer (&ibDesc, &ibInitialData, &indexBuffer_);
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