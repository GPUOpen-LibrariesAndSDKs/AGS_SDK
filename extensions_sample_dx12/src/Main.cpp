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
#include <DXGIDebug.h>
#include <stdexcept>

int WinMain (
    _In_ HINSTANCE,
    _In_opt_ HINSTANCE,
    _In_ LPSTR,
    _In_ int
    )
{
    // Enable run-time memory check for debug builds.
	// (When _DEBUG is not defined, calls to _CrtSetDbgFlag are removed during preprocessing.)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    AMD::D3D12Sample* sample = new AMD::ExtensionsSample12;

    if (sample == nullptr)
    {
        return 1;
    }
    
    try
    {
        sample->Run();
    }
    catch (const std::runtime_error& error)
    {
        MessageBoxA(0, error.what(), "Error", 0);
    }

    delete sample;

    //check refleaks
    HMODULE dxgidebug = LoadLibraryEx("dxgidebug.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (dxgidebug)
    {
        typedef HRESULT(WINAPI * LPDXGIGETDEBUGINTERFACE)(REFIID, void **);
        auto dxgiGetDebugInterface = reinterpret_cast<LPDXGIGETDEBUGINTERFACE>(
            reinterpret_cast<void*>(GetProcAddress(dxgidebug, "DXGIGetDebugInterface")));

        IDXGIDebug* debugController;
        if (dxgiGetDebugInterface(IID_PPV_ARGS(&debugController)) == S_OK)
        {
            debugController->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
            debugController->Release();
        }
    }

    return 0;
}
