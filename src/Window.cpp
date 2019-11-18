#include "stdafx.h"
#include "Window.h"
#include "WindowsHelpers.h"


Window::Window()
{
	MakeWindow();
}

Window::~Window()
{
}

bool Window::RegesterWindowClass(HINSTANCE instance, WNDPROC windowMessageProc)
{
	WNDCLASSEX wcx;

	// Fill in the window class structure with parameters 
	// that describe the main window. 

	wcx.cbSize = sizeof(wcx);          // size of structure 
	wcx.style = CS_HREDRAW |
		CS_VREDRAW;                    // redraw if size changes 
	wcx.lpfnWndProc = windowMessageProc;
	wcx.cbClsExtra = 0;                // no extra class memory 
	wcx.cbWndExtra = 0;                // no extra window memory 
	wcx.hInstance = instance;          // handle to instance 
	wcx.hIcon = LoadIcon(nullptr,
		IDI_APPLICATION);              // predefined app. icon 
	wcx.hCursor = LoadCursor(nullptr,
		IDC_ARROW);                    // predefined arrow 
	wcx.hbrBackground = GetSysColorBrush(
		WHITE_BRUSH);                   // white background brush 
	wcx.lpszMenuName = L"DXWinMenu";    // name of menu resource 
	wcx.lpszClassName = L"DXWClass";    // name of window class 
	wcx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// Register the window class. 
	if (FAILED(RegisterClassEx(&wcx)))
	{
		PrintLastErrorMessage(L"RegisterClassEx");
		return false;
	}

	return true;
}

void Window::MakeWindow()
{
	HINSTANCE instance = GetModuleHandle(nullptr);

	m_Window = CreateWindow(L"DXWClass", L"DXWindow",
		WS_OVERLAPPEDWINDOW, // top-level window 
		CW_USEDEFAULT,       // default horizontal position 
		CW_USEDEFAULT,       // default vertical position 
		CW_USEDEFAULT,       // default width 
		CW_USEDEFAULT,       // default height 
		(HWND)nullptr,       // no owner window 
		(HMENU)nullptr,      // use class menu 
		instance,            // handle to application instance 
		(LPVOID)nullptr);    // no window-creation data 

	if (!m_Window)
	{
		PrintLastErrorMessage(L"CreateWindow");
	}
}
