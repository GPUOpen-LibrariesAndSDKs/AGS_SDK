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
#ifndef AMD_AGS_CFX_API_SAMPLE_MAIN_H
#define AMD_AGS_CFX_API_SAMPLE_MAIN_H

#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <vector>

#include "amd_ags.h"

namespace AMD {
class Window;

///////////////////////////////////////////////////////////////////////////////
class CFXAPISample
{
private:
public:

	CFXAPISample ();
	~CFXAPISample ();

	void Run (const int frameCount);

protected:
	IDXGISwapChain*                             m_swapChain = nullptr;
	ID3D11Device*                               m_device = nullptr;
	ID3D11DeviceContext*                        m_deviceContext = nullptr;
	ID3D11Texture2D*                            m_renderTarget = nullptr;
	ID3D11RenderTargetView*                     m_renderTargetView = nullptr;

	ID3D11InputLayout*                          m_inputLayout = nullptr;
	ID3D11PixelShader*                          m_pixelShader = nullptr;
	ID3D11VertexShader*                         m_vertexShader = nullptr;

	ID3D11DepthStencilState*                    m_depthStencilState = nullptr;

	ID3D11Texture2D*                            m_texture = nullptr;
	ID3D11ShaderResourceView*                   m_textureSRV = nullptr;
	ID3D11RenderTargetView*                     m_textureRTV = nullptr;

	ID3D11Texture2D*                            m_uploadTexture = nullptr;
	ID3D11ShaderResourceView*                   m_uploadTextureSRV = nullptr;

private:
	void Initialize();
	void Shutdown();

	void PrepareRender();
	void FinalizeRender();

	void Render();
	void Present();
	void CreateDeviceAndSwapChain();
	void CreateMeshBuffers();

	std::unique_ptr<Window> m_window;

	ID3D11Buffer*                            m_vertexBuffer = nullptr;
	ID3D11Buffer*                            m_indexBuffer = nullptr;

	AGSContext*                             m_agsContext = nullptr;
    AGSGPUInfo                              m_agsGPUInfo = {};
	bool                                    m_cfxEnabled = false;
};
}

#endif