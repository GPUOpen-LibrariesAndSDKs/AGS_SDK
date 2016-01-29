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
#ifndef AMD_AGS_SDK_CFX_API_SAMPLE_MAIN_H_
#define AMD_AGS_SDK_CFX_API_SAMPLE_MAIN_H_

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
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
	Microsoft::WRL::ComPtr<ID3D11Device> device_;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext_;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> renderTarget_;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState_;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV_;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> textureRTV_;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> uploadTexture_;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> uploadTextureSRV_;

private:
	void Initialize ();
	void Shutdown ();

	void PrepareRender ();
	void FinalizeRender ();

	void Render ();
	void Present ();
	void CreateDeviceAndSwapChain ();
	void CreateMeshBuffers ();
	void InitializeAMDCFXAPI ();

	std::unique_ptr<Window> window_;

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;

	AGSContext*						agsContext_;
	bool							cfxEnabled_;
};
}

#endif