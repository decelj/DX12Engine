#include "stdafx.h"
#include "Engine.h"
#include "Window.h"
#include "DXDevice.h"
#include "DXCommandList.h"
#include "Utils.h"
#include "DXCompiler.h"
#include "RootSignatureBuilder.h"
#include "PSOBuilder.h"

#include "../OBJLoader/OBJLoader.h"

#include <d3d12.h>


Engine::Engine(const Window& window)
	: m_Camera(window.Width(), window.Height(), 50.f, 100.f)
	, m_FrameConstantData({})
	, m_Frame(0u)
{
	m_DepthBuffer = std::make_unique<DepthTarget>(DXGI_FORMAT_D24_UNORM_S8_UINT, window.Width(), window.Height());
	m_BackBuffers = DXDevice::Instance().CreateSwapChainTargets();
	m_CommandList = std::make_unique<DXCommandList>(CommandType::GRAPHICS);

	for (auto& constantBuffer : m_FrameConstBuffers)
	{
		constantBuffer = std::make_unique<ConstantBuffer>((uint32_t)sizeof(GlobalFrameConstatns));
	}

	LoadGeometry();
	LoadShaders();

	{
		PSOBuilder psoBuilder;
		psoBuilder.SetRootSignature(m_PrimaryRootSignature.get());
		psoBuilder.SetPixelShader(m_PixelShader.get());
		psoBuilder.SetVertexShader(m_VertexShader.get());
		psoBuilder.SetVertexLayout(VertexLayout::PositionUVNormal);
		psoBuilder.SetRTVFormats<1>({ DXGI_FORMAT_R8G8B8A8_UNORM });
		psoBuilder.SetDSVFormat(DXGI_FORMAT_D24_UNORM_S8_UINT);

		m_PrimaryPSO.reset(psoBuilder.Build());
	}

	m_UploadManager.MakeAllResident();
	m_UploadManager.WaitForUpload();

	m_Camera.LookAt({ 0.f, 1.f, 4.f }, { 0.f, 0.75f, 0.f });
}

void Engine::RenderFrame()
{
	const uint8_t bufferIdx = m_Frame % kNumFramesInFlight;

	std::unique_ptr<RenderTarget>& backBuffer = m_BackBuffers[bufferIdx];
	std::unique_ptr<ConstantBuffer>& frameConstBuffer = m_FrameConstBuffers[bufferIdx];

	m_FrameConstantData.proj = m_Camera.Proj();
	m_FrameConstantData.view = m_Camera.View();
	m_FrameConstantData.viewProj = m_Camera.Proj() * m_Camera.View();
	frameConstBuffer->SetData(0u, m_FrameConstantData);

	for (std::unique_ptr<Geometry>& geo : m_SceneGeometry)
	{
		geo->SetPosition({ std::sinf((float)m_Frame / 2000.f), 0.f, 0.f });
	}

	m_CommandList->Begin();

	m_Camera.SetViewport(*m_CommandList);
	m_CommandList->Native()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const glm::vec4 clearColor = glm::vec4(0.f);
	backBuffer->TransitionTo(ResourceState::RenderTarget, *m_CommandList);
	m_CommandList->Native()->ClearRenderTargetView(backBuffer->RTVHandle().cpuHandle, &clearColor.x, 0u, nullptr);
	m_CommandList->Native()->ClearDepthStencilView(m_DepthBuffer->DSVHandle().cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
	m_CommandList->Native()->OMSetRenderTargets(1, &backBuffer->RTVHandle().cpuHandle, true, &m_DepthBuffer->DSVHandle().cpuHandle);

	m_CommandList->Native()->SetGraphicsRootSignature(m_PrimaryRootSignature.get());
	m_CommandList->Native()->SetGraphicsRootConstantBufferView(0u, frameConstBuffer->GetGPUAddress());
	m_CommandList->Native()->SetPipelineState(m_PrimaryPSO.get());

	for (std::unique_ptr<Geometry>& geo : m_SceneGeometry)
	{
		geo->Draw(*m_CommandList);
		geo->NextFrame();
	}

	backBuffer->TransitionTo(ResourceState::Present, *m_CommandList);
	m_CommandList->End();
	m_CommandList->Submit();
	DXDevice::Instance().Present();

	++m_Frame;
}

void Engine::LoadGeometry()
{
	HighResTimer timer;
	timer.Start();

	std::vector<OBJLoader::Vertex> vertexData;
	std::vector<uint32_t> indexData;
	OBJLoader::LoadFile(L"models/bunny.obj", &vertexData, &indexData);

	m_SceneGeometry.emplace_back(std::make_unique<RigidGeometry>(std::move(vertexData), std::move(indexData), m_UploadManager));

	std::string loadTime = "Loaded geometry in " + timer.ElapsedAsString(timer.Elapsed()) + "\n";
	OutputDebugStringA(loadTime.c_str());
}

void Engine::LoadShaders()
{
	m_VertexShader.reset(
		DXCompiler::Instance().CompileShaderFromFile(
			L"Basic.hlsl", "VSMain", "vs",
			{
				{"HAS_NORMAL", "1"},
				{}
			}
	));
	m_PixelShader.reset(
		DXCompiler::Instance().CompileShaderFromFile(
			L"Basic.hlsl", "PSMain", "ps",
			{
				{"HAS_NORMAL", "1"},
				{}
			}
	));

	RootSignatureBuilder rootBuilder;
	rootBuilder.SetCBV(0, 0, 0);
	rootBuilder.SetCBV(1, 1, 0);
	m_PrimaryRootSignature.reset(rootBuilder.Build());
}
