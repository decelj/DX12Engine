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

		float		m_ConeAngle;
		glm::vec3	m_LightPostion;
	};

private:
};


class SpotLight : public Light
{
public:
	SpotLight(const glm::vec3& position, const glm::vec3& lookAt, const glm::vec3& color, float angleDegrees, uint32_t shadowMapSize, float nearClip, float farClip);
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
	glm::mat4 MakeView() const
	{
		glm::vec3 upVec = { 0.f, 1.f, 0.f };

		glm::vec3 sideVec = glm::cross(m_Direction, upVec);
		if (glm::all(glm::lessThan(glm::abs(sideVec), glm::vec3(0.01f))))
		{
			upVec = { 0.f, 0.f, 1.f };
			sideVec = glm::cross(m_Direction, upVec);
		}

		sideVec = glm::normalize(sideVec);
		upVec = glm::cross(sideVec, m_Direction);

		glm::mat4 view = glm::identity<glm::mat4>();
		view[0].x = sideVec.x;
		view[1].x = sideVec.y;
		view[2].x = sideVec.z;
		view[0].y = upVec.x;
		view[1].y = upVec.y;
		view[2].y = upVec.z;
		view[0].z = -m_Direction.x;
		view[1].z = -m_Direction.y;
		view[2].z = -m_Direction.z;
		view[3].x = -glm::dot(sideVec, m_Position);
		view[3].y = -glm::dot(upVec, m_Position);
		view[3].z =  glm::dot(m_Direction, m_Position);

		return view;
	}

	glm::mat4 MakeProjection() const
	{
		float viewportSize = m_ShadowMapSize ? (float)m_ShadowMapSize : 256.f;
		return glm::perspectiveFov(m_ConeAngle, viewportSize, viewportSize, m_NearClip, m_FarClip);
	}

	glm::vec3									m_Position;
	glm::vec3									m_Direction;
	float										m_ConeAngle;
	glm::vec3									m_LightColor;

	uint32_t									m_ShadowMapSize;
	float										m_NearClip;
	float										m_FarClip;

	std::unique_ptr<DepthTarget>				m_ShadowMap;
	IFFArray<std::unique_ptr<ConstantBuffer>>	m_ShadowMapConstants;
	IFFArray<std::unique_ptr<ConstantBuffer>>	m_LightConstants;
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
