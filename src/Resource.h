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

enum class ResourceFlags : uint32_t
{
	None = D3D12_RESOURCE_FLAG_NONE,
	RenderTarget = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
	DepthStencil = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
	UAV = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
	NonShaderResource = D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE,
	CrossAdapter = D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER,
	SimultaneousAccess = D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS,
	VideoDecodeReferenceOnly = D3D12_RESOURCE_FLAG_VIDEO_DECODE_REFERENCE_ONLY,
	CPUMappable = (1u << 31u)
};

class Resource
{
public:
	Resource(ResourceDimension dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, ResourceState initialState, ResourceFlags flags);
	~Resource() = default;

	void TransitionTo(ResourceState destState, DXCommandList& cmdList);
	ID3D12Resource* Native() { return m_Resource.get(); }
	D3D12_RESOURCE_DESC GetDesc() const { return m_Resource->GetDesc(); }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_Resource->GetGPUVirtualAddress(); }

protected:
	// Render target uses this constructor when creating objects
	// from the swap chain buffers
	Resource(ResourceState initialState);

	friend class UploadManager;
	void SetStateTo(ResourceState destState) { m_CurrentState = destState; }

	uint8_t* Map();
	void Unmap() { m_Resource->Unmap(0, nullptr); }

	ReleasedUniquePtr<ID3D12Resource>	m_Resource;
	ResourceState						m_CurrentState;
};
