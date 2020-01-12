// DX12Engine.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Windowsx.h"
#include "Window.h"
#include "DXDevice.h"
#include "DXCompiler.h"
#include "Engine.h"


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	if (!Window::RegisterWindowClass(hInstance, WndProc))
	{
		return -1;
	}

	Window window;
	DXDevice::InitFromWindow(window);
	window.Show(nCmdShow);

	DXCompiler::Initialize();
	DXCompiler::Instance().AddSearchPath(L"src/Shaders");

	MSG msg = { 0 };
	{
		Engine engine = Engine(window);
		
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				// Skip repeated key down messages when key is held
				if (msg.message == WM_KEYDOWN)
				{
					if ((msg.lParam >> 30u) & 0x1)
					{
						continue;
					}
				}

				TranslateMessage(&msg);

				switch (msg.message)
				{
				case WM_LBUTTONDOWN:
					SetCapture(window.Handle());
					engine.OnMousePress({ GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) });
					break;
				case WM_LBUTTONUP:
					ReleaseCapture();
					engine.OnMouseRelease({ GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) });
					break;
				case WM_MOUSEMOVE:
					engine.OnMouseMove({ GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam) });
					break;
				case WM_CHAR:
					if (msg.wParam < 256u)
					{
						/* WM_DEADCHAR doesn't seem to work.
						 * Catching WM_KEYUP instead, which is pre-translation.
						 * This means chars always uppercase.
						 * I should fix this someday...
						 */
						engine.OnKeyDown((unsigned char)std::toupper((int)msg.wParam));
					}
					break;
				case WM_KEYUP:
					if (msg.wParam < 256u)
					{
						engine.OnKeyUp((unsigned char)msg.wParam);
					}
					break;
				};

				DispatchMessage(&msg);
			}
			else
			{
				engine.RenderFrame();
			}
		}
	}

	DXCompiler::Destory();
	DXDevice::Destroy();

    return (int) msg.wParam;
}
