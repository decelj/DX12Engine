#pragma once

#include "stdafx.h"
#include "RenderTarget.h"
#include "Camera.h"
#include "Buffer.h"
#include "DXCommandList.h"

#include <d3d12.h>

class ShadowMapRenderer;


class Light
{
public:
	Light() = default;
	virtual ~Light() = default;
	Light(const Light& other) = delete;

	virtual bool HasShadowMap() const = 0;
	virtual const DepthTarget& ShadowMap() const = 0;
	virtual void SetupShadowMap(ShadowMapRenderer& renderer) const = 0;
	virtual void SetupLightingRender(DXCommandList& cmdList, uint32_t frameIdx) const = 0;

protected:
	struct ShadowMapViewConstants
	{
		glm::mat4	m_ViewProj;
	};

	struct LightConstants
	{
		glm::mat4	m_WorldToLight;
		glm::vec4	m_LightDirection;
		glm::vec4	m_LightColor;
	};

private:
};


class SpotLight : public Light
{
public:
	SpotLight(const glm::vec3& position, const glm::vec3& lookAt, const glm::vec3& color, float angleDegrees, uint32_t shadowMapSize, float nearFlip, float farClip);
	~SpotLight() = default;

	bool HasShadowMap() const override
	{
		return m_ShadowMap != nullptr;
	}

	const DepthTarget& ShadowMap() const override
	{
		return *m_ShadowMap;
	}

	void SetupShadowMap(ShadowMapRenderer& renderer) const override;
	void SetupLightingRender(DXCommandList& cmdList, uint32_t frameIdx) const override;

private:
	Camera										m_View;
	std::unique_ptr<DepthTarget>				m_ShadowMap;
	IFFArray<std::unique_ptr<ConstantBuffer>>	m_ShadowMapConstants;
	IFFArray<std::unique_ptr<ConstantBuffer>>	m_LightConstants;
	glm::vec3									m_LightColor;
	glm::vec3									m_Direction; // TODO: Hack
};


class ShadowMapRenderer
{
public:
	ShadowMapRenderer();
	~ShadowMapRenderer() = default;
	ShadowMapRenderer(const ShadowMapRenderer& other) = delete;

	uint32_t FrameIndex() const { return m_FrameIndex; }

	DXCommandList& CommandList() { return m_CommandList; }

	template<typename ContainerT>
	void RenderLight(const Light& light, const ContainerT& geometry)
	{
		Begin();

		light.SetupShadowMap(*this);
		for (const auto& mesh : geometry)
		{
			mesh->Draw(m_CommandList);
		}
	}

	void SubmitCommands();

private:
	void Begin();

	uint32_t								m_FrameIndex;
	DXCommandList							m_CommandList;
	ReleasedUniquePtr<ID3D12RootSignature>	m_RootSignature;
	ReleasedUniquePtr<ID3D12PipelineState>	m_PSO;
	ReleasedUniquePtr<ID3DBlob>				m_ShadowsVertexShader;
};
