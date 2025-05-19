//
// Copyright (c) 2025 Advanced Micro Devices, Inc. All rights reserved.
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
#include "Window.h"

namespace AMD {
namespace {
///////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK cfxWindowWndProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    auto ptr = ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    auto window = reinterpret_cast<IWindow*> (ptr);

    switch (uMsg) {
    case WM_CLOSE:
        window->OnClose();
        return 0;
    }

    return ::DefWindowProcA(hwnd, uMsg, wParam, lParam);
}
}

///////////////////////////////////////////////////////////////////////////////
IWindow::IWindow ()
{
}

///////////////////////////////////////////////////////////////////////////////
IWindow::~IWindow()
{
}

///////////////////////////////////////////////////////////////////////////////
int IWindow::GetWidth() const
{
    return GetWidthImpl();
}

///////////////////////////////////////////////////////////////////////////////
int IWindow::GetHeight() const
{
    return GetHeightImpl();
}

///////////////////////////////////////////////////////////////////////////////
Window::Window(const std::string& title, const int width, const int height)
    : isClosed_ (false)
    , width_ (width)
    , height_ (height)
{
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    ::RECT rect;
    ::SetRect(&rect, 0, 0, width, height);
    ::AdjustWindowRect(&rect, style, FALSE);

    windowClass_.reset(new WindowClass("CFX API Test Window", cfxWindowWndProc));

    // Create the main window.
    hwnd_ = CreateWindowA(windowClass_->GetName().c_str(),
        title.c_str (),
        style, CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top, (HWND)NULL,
        (HMENU)NULL, NULL, (LPVOID)NULL);

    ::SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR> (this));

    // Show the window and paint its contents.
    ::ShowWindow(hwnd_, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd_);
}

///////////////////////////////////////////////////////////////////////////////
bool IWindow::IsClosed() const
{
    return IsClosedImpl();
}

///////////////////////////////////////////////////////////////////////////////
HWND Window::GetHWND () const
{
    return hwnd_;
}

/////////////////////////////////////////////////////////////////////////
WindowClass::WindowClass(const std::string& name, ::WNDPROC procedure)
    : name_(name)
{
    ::WNDCLASSA wc;

    // Register the window class for the main window.
    wc.style = 0;
    wc.lpfnWndProc = procedure;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = NULL;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = name_.c_str();

    ::RegisterClassA(&wc);
}

/////////////////////////////////////////////////////////////////////////
const std::string& WindowClass::GetName() const
{
    return name_;
}

/////////////////////////////////////////////////////////////////////////
WindowClass::~WindowClass()
{
    ::UnregisterClassA(name_.c_str(),
        (HINSTANCE)::GetModuleHandle(NULL));
}
}