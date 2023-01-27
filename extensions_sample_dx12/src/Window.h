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

#ifndef AMD_BARYCENTRICS_WINDOW_H_
#define AMD_BARYCENTRICS_WINDOW_H_

#include <string>
#include <Windows.h>
#include <memory>

namespace AMD
{
/**
* Encapsulate a window class.
*
* Calls <c>::RegisterClass()</c> on create, and <c>::UnregisterClass()</c>
* on destruction.
*/
struct WindowClass final
{
public:
    WindowClass (const std::string& name, WNDPROC procedure = ::DefWindowProc);
    ~WindowClass ();

    const std::string& GetName () const;

    WindowClass (const WindowClass&) = delete;
    WindowClass& operator= (const WindowClass&) = delete;

private:
    std::string m_name;
};

struct IWindow
{
public:
    IWindow () = default;
    IWindow (const IWindow&) = delete;
    IWindow& operator=(const IWindow&) = delete;

    virtual ~IWindow ();

    bool IsClosed () const;
    virtual void OnClose () = 0;
    virtual bool MessagePump() const = 0;
    int GetWidth () const;
    int GetHeight () const;

private:
    virtual bool IsClosedImpl () const = 0;
    virtual int GetWidthImpl () const = 0;
    virtual int GetHeightImpl () const = 0;
};

class Window : public IWindow
{
public:
    Window (const std::string& title, int width, int height);

    HWND GetHWND () const;

    void OnClose () override
    {
        m_isClosed = true;
    }

    bool MessagePump() const;

private:
    bool IsClosedImpl () const override
    {
        return m_isClosed;
    }

    int GetWidthImpl () const override
    {
        return m_width;
    }

    int GetHeightImpl () const override
    {
        return m_height;
    }

    std::unique_ptr<WindowClass> m_windowClass;
    HWND m_hwnd = 0;
    bool m_isClosed = false;
    int m_width = -1, m_height = -1;
};
}   // namespace AMD

#endif
