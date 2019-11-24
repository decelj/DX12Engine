#pragma once

#include "stdafx.h"
#include "ResourceState.h"

#include <d3d12.h>

class DXCommandList;

enum class ResourceDimension
{
	Linear,
	Texture2D,
	Texture3D
};

class Resource
{
public:
	Resource(ResourceDimension dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, ResourceState initialState);
	~Resource() = default;

	void TransitionTo(ResourceState destState, DXCommandList& cmdList);
	ID3D12Resource* Native() { return m_Resource.get(); }
	D3D12_RESOURCE_DESC GetDesc() const { return m_Resource->GetDesc(); }

protected:
	// Render target uses this constructor when creating objects
	// from the swap chain buffers
	Resource(ResourceState initialState);

	friend class UploadManager;
	void SetStateTo(ResourceState destState) { m_CurrentState = destState; }

	ReleasedUniquePtr<ID3D12Resource>	m_Resource;
	ResourceState						m_CurrentState;
};
