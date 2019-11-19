#pragma once

#include "stdafx.h"
#include "DescriptorHeap.h"
#include "ResourceState.h"

#include <d3d12.h>

class DXDevice;
class DXCommandList;

class RenderTarget
{
public:
	RenderTarget(DXDevice& device, DXGI_FORMAT format, uint32_t width, uint32_t height);
	RenderTarget(DXDevice& device, ID3D12Resource* texture, ResourceState currentState);
	RenderTarget(const RenderTarget& other) = delete;
	~RenderTarget();

	const DescriptorHandle& RTVHandle() const { return m_RTVHandle; }

	void TransitionTo(ResourceState destState, DXCommandList& cmdList);

private:
	ReleasedUniquePtr<ID3D12Resource>	m_Texture;
	DescriptorHandleWithIdx				m_RTVHandle;
	ResourceState						m_CurrentState;

	DXGI_FORMAT							m_Format;
	uint32_t							m_Width;
	uint32_t							m_Height;

	const bool							m_IsOwner;
};
