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
#include "Utils.h"
#include "VertexLayoutMngr.h"

#include "../OBJLoader/OBJLoader.h"


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
		DepthTarget depthBuffer(DXGI_FORMAT_D24_UNORM_S8_UINT, window.Width(), window.Height());

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

		HighResTimer timer;
		timer.Start();
		std::vector<OBJLoader::Vertex> bunnyVertexData;
		std::vector<uint32_t> bunnyIndiciesData;
		OBJLoader::LoadFile(L"models/bunny.obj", &bunnyVertexData, &bunnyIndiciesData);
		{
			std::string loadTime = "Loaded bunny.obj in " + timer.ElapsedAsString(timer.Elapsed()) + "\n";
			OutputDebugStringA(loadTime.c_str());
		}

		VertexBuffer bunnyVerts(sizeof(OBJLoader::Vertex) * bunnyVertexData.size(), sizeof(OBJLoader::Vertex));
		IndexBuffer bunnyIdicies(bunnyIndiciesData.size());
		uploadMngr.UploadDataTo(bunnyVertexData.data(), bunnyVertexData.size() * sizeof(OBJLoader::Vertex), bunnyVerts);
		uploadMngr.UploadDataTo(bunnyIndiciesData.data(), bunnyIndiciesData.size() * sizeof(uint32_t), bunnyIdicies);
		bunnyIndiciesData.clear();
		bunnyVertexData.clear();

		uploadMngr.MakeAllResident();
		uploadMngr.WaitForUpload();

		cmdList.Begin();
		triVerts.TransitionTo(ResourceState::VertexAndConstantBuffer, cmdList);
		triIndicies.TransitionTo(ResourceState::IndexBuffer, cmdList);
		cmdList.End();
		cmdList.Submit();

		ReleasedUniquePtr<ID3DBlob> vertexShader(DXCompiler::Instance().CompileShaderFromFile(L"Basic.hlsl", "VSMain", "vs", {}));
		ReleasedUniquePtr<ID3DBlob> pixelShader(DXCompiler::Instance().CompileShaderFromFile(L"Basic.hlsl", "PSMain", "ps", {}));

		ReleasedUniquePtr<ID3DBlob> vertexShaderWNormal(
			DXCompiler::Instance().CompileShaderFromFile(L"Basic.hlsl", "VSMain", "vs", { {"HAS_NORMAL", "1"}, {} }));
		ReleasedUniquePtr<ID3DBlob> pixelShaderWNormal(
			DXCompiler::Instance().CompileShaderFromFile(L"Basic.hlsl", "PSMain", "ps", { {"HAS_NORMAL", "1"}, {} }));

		ReleasedUniquePtr<ID3D12PipelineState> pso = nullptr;
		ReleasedUniquePtr<ID3D12PipelineState> psoNormal = nullptr;
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
			psoBuilder.SetVertexLayout(VertexLayout::PositionUV);
			psoBuilder.SetRTVFormats<1>({ DXGI_FORMAT_R8G8B8A8_UNORM });
			psoBuilder.SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);
			pso.reset(psoBuilder.Build());

			psoBuilder.SetVertexShader(vertexShaderWNormal.get());
			psoBuilder.SetPixelShader(pixelShaderWNormal.get());
			psoBuilder.SetVertexLayout(VertexLayout::PositionUVNormal);
			psoNormal.reset(psoBuilder.Build());
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

				cam.LookAt(
					{ 
						0.f,//std::sinf((float)frame / 2000.f) * 4.f,
						1.f,
						4.f,//std::cosf((float)frame / 2000.f) * 4.f
					},
					{ 0.f, 0.75f, 0.f });
				frameConsts.view = cam.View();
				frameConsts.proj = cam.Proj();
				frameConsts.viewProj = cam.Proj() * cam.View();
				frameConsts.aspect = aspect;
				frameConsts.offset.x = std::fabsf(std::sinf((float)frame / 1024.f));
				frameConsts.offset.y = std::fabsf(std::cosf((float)frame / 1024.f));
				frameConstBuffer->SetData(0, frameConsts);

				cmdList.Begin();

				cam.SetViewport(cmdList);

				D3D12_RECT scissorRect = { 0, 0, (LONG)window.Width(), (LONG)window.Height() };
				cmdList.Native()->RSSetScissorRects(1, &scissorRect);
				cmdList.Native()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				backBuffer->TransitionTo(ResourceState::RenderTarget, cmdList);
				cmdList.Native()->ClearRenderTargetView(backBuffer->RTVHandle().cpuHandle, clearColor.data(), 0, nullptr);
				cmdList.Native()->ClearDepthStencilView(depthBuffer.DSVHandle().cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
				cmdList.Native()->OMSetRenderTargets(1, &backBuffer->RTVHandle().cpuHandle, true, &depthBuffer.DSVHandle().cpuHandle);
				
				cmdList.Native()->SetGraphicsRootSignature(rootSig.get());
				cmdList.Native()->SetGraphicsRootConstantBufferView(0, frameConstBuffer->GetGPUAddress());

#if 0
				cmdList.Native()->SetPipelineState(pso.get());
				cmdList.Native()->IASetVertexBuffers(0, 1, &triVerts.View());
				cmdList.Native()->IASetIndexBuffer(&triIndicies.View());
				cmdList.Native()->DrawIndexedInstanced(triIndicies.Count(), 1, 0, 0, 0);
#endif

				cmdList.Native()->SetPipelineState(psoNormal.get());
				cmdList.Native()->IASetIndexBuffer(&bunnyIdicies.View());
				cmdList.Native()->IASetVertexBuffers(0, 1, &bunnyVerts.View());
				cmdList.Native()->DrawIndexedInstanced(bunnyIdicies.Count(), 1, 0, 0, 0);

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
