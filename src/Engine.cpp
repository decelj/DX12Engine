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
	, m_CameraSpeed(0.005f)
	, m_CameraVelocity({})
	, m_ClickPos({-1, -1})
{
	m_DepthBuffer = std::make_unique<DepthTarget>(DXGI_FORMAT_D24_UNORM_S8_UINT, window.Width(), window.Height());
	m_BackBuffers = DXDevice::Instance().CreateSwapChainTargets();
	m_CommandList = std::make_unique<DXCommandList>(CommandType::GRAPHICS);

	for (auto& constantBuffer : m_FrameConstBuffers)
	{
		constantBuffer = std::make_unique<ConstantBuffer>((uint32_t)sizeof(GlobalFrameConstatns));
	}

	{
		std::vector<glm::u8vec4> texData(32u * 32u);
		for (size_t y = 0; y < 32u; ++y)
		{
			const size_t rowOffset = y * 32ull;
			const bool rowLit = (y % 16u) / 8u;
			for (size_t x = 0u; x < 32u; ++x)
			{
				bool colLit = (x % 16u) / 8u;
				texData[rowOffset + x] = (colLit ^ rowLit) ? glm::u8vec4(255u) : glm::u8vec4(0u, 0u, 0u, 255u);
			}
		}

		m_CheckboardTexture = std::make_unique<Texture2D>(
			DXGI_FORMAT_R8G8B8A8_UNORM, 32u, 32u,
			texData.data(), (uint32_t)sizeof(glm::u8vec4),
			m_UploadManager);
	}

	LoadShaders();
	SetupLights();
	LoadGeometry();

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

	m_CommandList->Begin();
	m_CheckboardTexture->TransitionTo(ResourceState::PixelShaderResource | ResourceState::NonPixelShaderResource, *m_CommandList);
	m_CommandList->End();
	m_CommandList->Submit();

	m_Camera.LookAt({ 0.f, 1.f, 4.f }, { 0.f, 0.75f, 0.f });
}

void Engine::RenderFrame()
{
	for (const std::unique_ptr<Light>& light : m_Lights)
	{
		if (light->HasShadowMap())
		{
			m_ShadowMapRenderer.RenderLight(*light, m_SceneGeometry);
		}
	}

	m_ShadowMapRenderer.SubmitCommands();

	const uint8_t bufferIdx = m_Frame % kNumFramesInFlight;

	std::unique_ptr<RenderTarget>& backBuffer = m_BackBuffers[bufferIdx];
	std::unique_ptr<ConstantBuffer>& frameConstBuffer = m_FrameConstBuffers[bufferIdx];

	m_Camera.Translate(m_CameraVelocity);
	m_FrameConstantData.proj = m_Camera.Proj();
	m_FrameConstantData.view = m_Camera.View();
	m_FrameConstantData.viewProj = m_Camera.Proj() * m_Camera.View();
	frameConstBuffer->SetData(0u, m_FrameConstantData);

	for (std::unique_ptr<Geometry>& geo : m_SceneGeometry)
	{
		//geo->SetPosition({ std::sinf((float)m_Frame / 2000.f), 0.f, 0.f });
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

	// HACK
	m_Lights[0]->SetupLightingRender(*m_CommandList, bufferIdx);

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

void Engine::OnKeyDown(char key)
{
	switch (key)
	{
	case 'w':
	case 'W':
		m_CameraVelocity.z = m_CameraSpeed;
		break;
	case 's':
	case 'S':
		m_CameraVelocity.z = -m_CameraSpeed;
		break;
	case 'a':
	case 'A':
		m_CameraVelocity.x = m_CameraSpeed;
		break;
	case 'd':
	case 'D':
		m_CameraVelocity.x = -m_CameraSpeed;
		break;
	default:
		break;
	}
}

void Engine::OnKeyUp(char key)
{
	switch (key)
	{
	case 'w':
	case 'W':
	case 's':
	case 'S':
		m_CameraVelocity.z = 0.f;
		break;
	case 'a':
	case 'A':
	case 'd':
	case 'D':
		m_CameraVelocity.x = 0.f;
		break;
	default:
		break;
	}
}

void Engine::OnMousePress(const glm::ivec2& pos)
{
	m_ClickPos = pos;
}

void Engine::OnMouseRelease(const glm::ivec2& pos)
{
	m_ClickPos = { -1, -1 };
}

void Engine::OnMouseMove(const glm::ivec2& pos)
{
	if (m_ClickPos == glm::ivec2(-1, -1))
	{
		return;
	}

	glm::vec2 delta = pos - m_ClickPos;
	m_ClickPos = pos;
	delta = glm::radians(delta);

	m_Camera.RotateX(delta.y);
	m_Camera.RotateY(delta.x);
}

void Engine::LoadGeometry()
{
	HighResTimer timer;
	timer.Start();

	std::vector<OBJLoader::Vertex> vertexData;
	std::vector<uint32_t> indexData;
	OBJLoader::LoadFile(L"models/bunny.obj", &vertexData, &indexData);

	m_SceneGeometry.emplace_back(std::make_unique<RigidGeometry>(std::move(vertexData), std::move(indexData), m_UploadManager));

	{
		static constexpr OBJLoader::Vertex planeVerts[] = {
			{{-10.f, 0.f,  10.f}, {0.f, 1.f, 0.f}, {0.f, 0.f}},
			{{ 10.f, 0.f,  10.f}, {0.f, 1.f, 0.f}, {1.f, 0.f}},
			{{ 10.f, 0.f, -10.f}, {0.f, 1.f, 0.f}, {1.f, 1.f}},
			{{-10.f, 0.f, -10.f}, {0.f, 1.f, 0.f}, {0.f, 1.f}},
		};

		static constexpr uint32_t planeIndicies[] = {
			0, 1, 2,
			0, 2, 3
		};

		m_SceneGeometry.emplace_back(
			std::make_unique<StaticGeometry>(
				planeVerts, (uint32_t)_countof(planeVerts), (uint32_t)sizeof(planeVerts[0]),
				planeIndicies, (uint32_t)_countof(planeIndicies),
				m_UploadManager));
	}

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
	rootBuilder.SetCBV(2, 2, 0);
	rootBuilder.SetTable<1>(3,
		{
			{RootSignatureBuilder::RangeType::SRV, 0, 0, 0},
		});

	rootBuilder.SetStaticSampler(0, 0, 0,
		D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_COMPARISON_FUNC_LESS);

	rootBuilder.SetStaticSampler(1, 1, 0,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_COMPARISON_FUNC_LESS);

	m_PrimaryRootSignature.reset(rootBuilder.Build());
}

void Engine::SetupLights()
{
	m_Lights.emplace_back(
		std::make_unique<SpotLight>(
			glm::vec3(3.f, 3.f, 3.f),
			glm::vec3(0.f, 0.5f, 0.f),
			glm::vec3(1.f, 0.6f, 0.6f),
			40.f, 512u, 0.1f, 30.f));
}
