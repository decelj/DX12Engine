// DX12Engine.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DX12Engine.h"
#include "Window.h"
#include "DXDevice.h"
#include "DXCompiler.h"
#include "DXCommandList.h"
#include "RenderTarget.h"
#include "UploadManager.h"
#include "PSOBuilder.h"
#include "RootSignatureBuilder.h"
#include "Buffer.h"
#include "Camera.h"


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
	window.Show(nCmdShow);

	DXCompiler::Initialize();
	DXCompiler::Instance().AddSearchPath(L"src/Shaders");

	MSG msg = { 0 };
	{
		Camera cam(window.Width(), window.Height(), 50.f, 100.f);
		cam.LookAt({ 0.f, 0.f, -1.f }, { 0.f, 0.f, 0.f });

		std::vector<std::unique_ptr<RenderTarget>> backBuffers = DXDevice::Instance().CreateSwapChainTargets();
		DXCommandList cmdList(CommandType::GRAPHICS);
		UploadManager uploadMngr;

		struct Vertex
		{
			float x;
			float y;
			float z;

			float u;
			float v;
		};

		float aspect = (float)window.Width() / (float)window.Height();
		Vertex triangleVertexData[] =
		{
			{  0.0f ,  0.25f, 0.0f, 1.0f, 1.0f },
			{  0.25f, -0.25f, 0.0f, 1.0f, 0.0f },
			{ -0.25f, -0.25f, 0.0f, 0.0f, 1.0f }
		};

		VertexBuffer triVerts(sizeof(triangleVertexData), sizeof(Vertex));
		uploadMngr.UploadDataTo((void*)triangleVertexData, sizeof(triangleVertexData), triVerts);

		uint32_t triangleIndicies[] =
		{
			0, 1, 2
		};
		IndexBuffer triIndicies(_countof(triangleIndicies));
		uploadMngr.UploadDataTo((void*)triangleIndicies, sizeof(triangleIndicies), triIndicies);

		uploadMngr.MakeAllResident();
		uploadMngr.WaitForUpload();

		cmdList.Begin();
		triVerts.TransitionTo(ResourceState::VertexAndConstantBuffer, cmdList);
		triIndicies.TransitionTo(ResourceState::IndexBuffer, cmdList);
		cmdList.End();
		cmdList.Submit();

		ReleasedUniquePtr<ID3DBlob> vertexShader(DXCompiler::Instance().CompileShaderFromFile(L"Basic.hlsl", "VSMain", "vs", {}));
		ReleasedUniquePtr<ID3DBlob> pixelShader(DXCompiler::Instance().CompileShaderFromFile(L"Basic.hlsl", "PSMain", "ps", {}));

		ReleasedUniquePtr<ID3D12PipelineState> pso = nullptr;
		ReleasedUniquePtr<ID3D12RootSignature> rootSig = nullptr;

		{
			RootSignatureBuilder rootBuilder;
			rootBuilder.SetCBV(0, 0, 0);
			rootSig.reset(rootBuilder.Build());
		}

		{
			PSOBuilder psoBuilder;
			psoBuilder.SetRootSignature(rootSig.get());
			psoBuilder.SetPixelShader(pixelShader.get());
			psoBuilder.SetVertexShader(vertexShader.get());
			psoBuilder.AppendInputElements(
				{
					{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0},
					{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, offsetof(Vertex, u)}
				});
			psoBuilder.SetRTVFormats<1>({ DXGI_FORMAT_R8G8B8A8_UNORM });
			pso.reset(psoBuilder.Build());
		}

		struct FrameConstants
		{
			glm::mat4 view;
			glm::mat4 proj;
			glm::mat4 viewProj;
			float aspect;
			glm::vec2 offset;
			float padding;
		};

		std::vector<std::unique_ptr<ConstantBuffer>> frameConstantBuffers(2);
		frameConstantBuffers[0].reset(new ConstantBuffer(sizeof(FrameConstants)));
		frameConstantBuffers[1].reset(new ConstantBuffer(sizeof(FrameConstants)));

		constexpr std::array<float, 4u> clearColor = { 0.f, 0.f, 0.f, 0.f };
		FrameConstants frameConsts = {};
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
				std::unique_ptr<ConstantBuffer>& frameConstBuffer = frameConstantBuffers[frame & 0x1];

				cam.LookAt({ std::sinf((float)frame / 2000.f), 0.f, -1.f }, { 0.f, 0.f, 0.f });
				frameConsts.view = cam.View();
				frameConsts.proj = cam.Proj();
				frameConsts.viewProj = cam.Proj() * cam.View();
				frameConsts.aspect = aspect;
				frameConsts.offset.x = std::fabsf(std::sinf((float)frame / 1024.f));
				frameConsts.offset.y = std::fabsf(std::cosf((float)frame / 1024.f));
				frameConstBuffer->SetData(0, frameConsts);

				cmdList.Begin();

				D3D12_VIEWPORT vp = { 0.f, 0.f, (float)window.Width(), (float)window.Height(), 0.f, 1.f };
				cmdList.Native()->RSSetViewports(1, &vp);
				D3D12_RECT scissorRect = { 0, 0, (LONG)window.Width(), (LONG)window.Height() };
				cmdList.Native()->RSSetScissorRects(1, &scissorRect);

				backBuffer->TransitionTo(ResourceState::RenderTarget, cmdList);
				cmdList.Native()->ClearRenderTargetView(backBuffer->RTVHandle().cpuHandle, clearColor.data(), 0, nullptr);

				cmdList.Native()->OMSetRenderTargets(1, &backBuffer->RTVHandle().cpuHandle, true, nullptr);
				cmdList.Native()->SetGraphicsRootSignature(rootSig.get());
				cmdList.Native()->SetGraphicsRootConstantBufferView(0, frameConstBuffer->GetGPUAddress());
				cmdList.Native()->SetPipelineState(pso.get());
				cmdList.Native()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				cmdList.Native()->IASetVertexBuffers(0, 1, &triVerts.View());
				cmdList.Native()->IASetIndexBuffer(&triIndicies.View());
				cmdList.Native()->DrawIndexedInstanced(triIndicies.Count(), 1, 0, 0, 0);

				backBuffer->TransitionTo(ResourceState::Present, cmdList);
				cmdList.End();
				cmdList.Submit();
				DXDevice::Instance().Present();

				++frame;
			}
		}
	}

	DXCompiler::Destory();
	DXDevice::Destroy();

    return (int) msg.wParam;
}
