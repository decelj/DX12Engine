// DX12Engine.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DX12Engine.h"
#include "Window.h"
#include "DXDevice.h"
#include "DXCompiler.h"
#include "DXCommandList.h"
#include "RenderTarget.h"


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

	if (!Window::RegesterWindowClass(hInstance, WndProc))
	{
		return -1;
	}

	Window window;
	DXDevice::InitFromWindow(window);
	DXCompiler::Initialize();
	window.Show(nCmdShow);

	MSG msg = { 0 };
	{
		std::vector<std::unique_ptr<RenderTarget>> backBuffers = DXDevice::Instance().CreateSwapChainTargets();
		DXCommandList cmdList(DXDevice::Instance());

		uint32_t frame = 0;
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				std::unique_ptr<RenderTarget>& backBuffer = backBuffers[frame & 0x1];

				std::array<float, 4u> clearColor = { 1.f - ((frame % 360) / 360.f), 0.f, ((frame % 360) / 360.f), 1.f };

				cmdList.Begin();
				backBuffer->TransitionTo(ResourceState::RenderTarget, cmdList);
				cmdList.Native()->ClearRenderTargetView(backBuffer->RTVHandle().cpuHandle, clearColor.data(), 0, nullptr);
				backBuffer->TransitionTo(ResourceState::Present, cmdList);
				cmdList.End();
				cmdList.Submit(DXDevice::Instance());
				DXDevice::Instance().Present();

				++frame;
			}
		}
	}

	DXCompiler::Destory();
	DXDevice::Destroy();

    return (int) msg.wParam;
}
