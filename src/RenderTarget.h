#pragma once

#include "stdafx.h"
#include "DescriptorHeap.h"
#include "Resource.h"

#include <d3d12.h>

class RenderTarget : public Resource
{
public:
	RenderTarget(ResourceDimension dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, const glm::vec4& clearColor);
	RenderTarget(ID3D12Resource* texture, ResourceState currentState);
	RenderTarget(const RenderTarget& other) = delete;
	~RenderTarget();

	const DescriptorHandle& RTVHandle() const { return m_RTVHandle; }

private:
	DescriptorHandleWithIdx				m_RTVHandle;

	const bool							m_IsOwner;
};

class DepthTarget : public Resource
{
public:
	DepthTarget(DXGI_FORMAT format, uint32_t width, uint32_t height, const float clearValue=1.f);
	DepthTarget(const DepthTarget& other) = delete;
	~DepthTarget();

	const DescriptorHandle& DSVHandle() const { return m_DSVHandle; }

private:
	DescriptorHandleWithIdx				m_DSVHandle;
};
