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
			{  0.0f ,  0.25f * aspect, 0.0f, 1.0f, 1.0f },
			{  0.25f, -0.25f * aspect, 0.0f, 1.0f, 0.0f },
			{ -0.25f, -0.25f * aspect, 0.0f, 0.0f, 1.0f }
		};

		VertexBuffer triVerts(DXGI_FORMAT_UNKNOWN, sizeof(triangleVertexData), sizeof(Vertex));
		uploadMngr.UploadDataTo((void*)triangleVertexData, sizeof(triangleVertexData), triVerts);
		uploadMngr.MakeAllResident();
		uploadMngr.WaitForUpload();

		cmdList.Begin();
		triVerts.TransitionTo(ResourceState::VertexAndConstantBuffer, cmdList);
		cmdList.End();
		cmdList.Submit();

		const std::string kShader = "struct PSInput \
		{ \
			float4 position : SV_POSITION; \
			float2 uv : TEXCOORD; \
		}; \
\
		Texture2D g_texture : register(t0); \
		SamplerState g_sampler : register(s0); \
\
		PSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD) \
		{ \
			PSInput result; \
\
			result.position = position; \
			result.uv = uv; \
\
			return result; \
		}\
\
		float4 PSMain(PSInput input) : SV_TARGET \
		{ \
			return float4(input.uv.x, input.uv.y, 0, 1); \
		}";

		ReleasedUniquePtr<ID3DBlob> vertexShader(DXCompiler::Instance().CompileShader(kShader.c_str(), "VSMain", "vs", {}));
		ReleasedUniquePtr<ID3DBlob> pixelShader(DXCompiler::Instance().CompileShader(kShader.c_str(), "PSMain", "ps", {}));

		ReleasedUniquePtr<ID3D12PipelineState> pso = nullptr;
		ReleasedUniquePtr<ID3D12RootSignature> rootSig = nullptr;

		{
			RootSignatureBuilder rootBuilder;
			rootSig.reset(rootBuilder.Build(DXDevice::Instance()));
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
			pso.reset(psoBuilder.Build(DXDevice::Instance()));
		}

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
				std::array<float, 4u> clearColor = { 0.f, 0.f, 0.f, 0.f };

				cmdList.Begin();

				D3D12_VIEWPORT vp = { 0.f, 0.f, (float)window.Width(), (float)window.Height(), 0.f, 1.f };
				cmdList.Native()->RSSetViewports(1, &vp);
				D3D12_RECT scissorRect = { 0, 0, (LONG)window.Width(), (LONG)window.Height() };
				cmdList.Native()->RSSetScissorRects(1, &scissorRect);

				backBuffer->TransitionTo(ResourceState::RenderTarget, cmdList);
				cmdList.Native()->ClearRenderTargetView(backBuffer->RTVHandle().cpuHandle, clearColor.data(), 0, nullptr);

				cmdList.Native()->OMSetRenderTargets(1, &backBuffer->RTVHandle().cpuHandle, true, nullptr);
				cmdList.Native()->SetGraphicsRootSignature(rootSig.get());
				cmdList.Native()->SetPipelineState(pso.get());
				cmdList.Native()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				cmdList.Native()->IASetVertexBuffers(0, 1, &triVerts.View());
				cmdList.Native()->DrawInstanced(3, 1, 0, 0);

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
