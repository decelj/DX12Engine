#include "stdafx.h"
#include "Light.h"
#include "DXCompiler.h"
#include "RootSignatureBuilder.h"
#include "PSOBuilder.h"

ShadowMapRenderer::ShadowMapRenderer()
	: m_FrameIndex(0u)
	, m_CommandList(CommandType::GRAPHICS)
	, m_RootSignature(nullptr)
	, m_PSO(nullptr)
	, m_ShadowsVertexShader(nullptr)
{
	m_ShadowsVertexShader.reset(
		DXCompiler::Instance().CompileShaderFromFile(
			L"ShadowMap.hlsl", "VSMain", "vs",
			{
				{"HAS_NORMAL", "1"},
				{}
			}
	));

	RootSignatureBuilder rootBuilder;
	rootBuilder.SetCBV(0, 0, 0);
	rootBuilder.SetCBV(1, 1, 0);
	m_RootSignature.reset(rootBuilder.Build());

	{
		PSOBuilder psoBuilder;
		psoBuilder.SetRootSignature(m_RootSignature.get());
		psoBuilder.SetVertexShader(m_ShadowsVertexShader.get());
		psoBuilder.SetVertexLayout(VertexLayout::PositionUVNormal);
		psoBuilder.SetDSVFormat(DXGI_FORMAT_D32_FLOAT);

		m_PSO.reset(psoBuilder.Build());
	}
}

void ShadowMapRenderer::Begin()
{
	if (m_CommandList.IsOpen())
	{
		return;
	}

	m_CommandList.Begin(m_PSO.get());
	m_CommandList.Native()->SetGraphicsRootSignature(m_RootSignature.get());
	m_CommandList.Native()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ShadowMapRenderer::SubmitCommands()
{
	m_CommandList.End();
	m_CommandList.Submit();

	m_FrameIndex = (m_FrameIndex + 1u) % kNumFramesInFlight;
}


SpotLight::SpotLight(const glm::vec3& position, const glm::vec3& lookAt, const glm::vec3& color, float angleDegrees, uint32_t shadowMapSize, float nearClip, float farClip)
	: m_Position(position)
	, m_Direction(glm::normalize(lookAt - position))
	, m_ConeAngle(glm::radians(angleDegrees))
	, m_LightColor(color)
	, m_ShadowMapSize(shadowMapSize)
	, m_NearClip(nearClip)
	, m_FarClip(farClip)
	, m_ShadowMap(nullptr)
{
	if (shadowMapSize > 0u)
	{
		m_ShadowMap = std::make_unique<DepthTarget>(DXGI_FORMAT_D32_FLOAT, shadowMapSize, shadowMapSize);
		for (auto& constantBuffer : m_ShadowMapConstants)
		{
			constantBuffer = std::make_unique<ConstantBuffer>((uint32_t)sizeof(ShadowMapViewConstants));
		}
	}

	for (auto& constantBuffer : m_LightConstants)
	{
		constantBuffer = std::make_unique<ConstantBuffer>((uint32_t)sizeof(LightConstants));
	}
}

void SpotLight::SetupShadowMap(ShadowMapRenderer& renderer) const
{
	DXCommandList& cmdList = renderer.CommandList();

	{
		ShadowMapViewConstants viewConstants;
		viewConstants.m_ViewProj = MakeProjection() * MakeView();
		m_ShadowMapConstants[renderer.FrameIndex()]->SetData(0u, viewConstants);
	}

	D3D12_VIEWPORT vp = { 0.f, 0.f, (float)m_ShadowMapSize, (float)m_ShadowMapSize, 0.f, 1.f };
	cmdList.Native()->RSSetViewports(1, &vp);

	D3D12_RECT scissorRect = { 0u, 0u, (LONG)m_ShadowMapSize, (LONG)m_ShadowMapSize };
	cmdList.Native()->RSSetScissorRects(1, &scissorRect);

	m_ShadowMap->TransitionTo(ResourceState::DepthWrite, cmdList);
	cmdList.Native()->ClearDepthStencilView(m_ShadowMap->DSVHandle().cpuHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
	cmdList.Native()->OMSetRenderTargets(0u, nullptr, false, &m_ShadowMap->DSVHandle().cpuHandle);
	cmdList.Native()->SetGraphicsRootConstantBufferView(0u, m_ShadowMapConstants[renderer.FrameIndex()]->GetGPUAddress());
}

void SpotLight::SetupLightingRender(DXCommandList& cmdList, uint32_t frameIdx) const
{
	{
		LightConstants constants;
		constants.m_LightColor = glm::vec4(m_LightColor, 0.f);
		constants.m_LightDirection = glm::vec4(m_Direction, 0.f);
		constants.m_LightPostion = m_Position;
		constants.m_ConeAngle = glm::cos(m_ConeAngle / 2.f);

		glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), { 0.5f, -0.5f, 1.f });
		glm::mat4 translate = glm::translate(glm::identity<glm::mat4>(), { 0.5f, 0.5f, 0.f });
		constants.m_WorldToLight = translate * scale * MakeProjection() * MakeView();

		m_LightConstants[frameIdx]->SetData(0u, constants);
	}

	m_ShadowMap->TransitionTo(ResourceState::PixelShaderResource | ResourceState::NonPixelShaderResource, cmdList);
	cmdList.Native()->SetGraphicsRootConstantBufferView(2u, m_LightConstants[frameIdx]->GetGPUAddress());
	cmdList.Native()->SetGraphicsRootDescriptorTable(3u, m_ShadowMap->SRVHandle().gpuHandle);
}
