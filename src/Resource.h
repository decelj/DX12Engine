#pragma once

#include "stdafx.h"
#include "DXFence.h"
#include "ResourceState.h"

#include <d3d12.h>

class DXDevice;
class RenderTarget;
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
	Resource(ResourceDimension dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, void* data, size_t dataSize, ResourceState initialState);
	~Resource() = default;

	void TransitionTo(ResourceState destState, DXCommandList& cmdList);
	void MakeResident();

protected:
	// Render target uses this constructor when creating objects
	// from the swap chain buffers
	Resource(ResourceState initialState);

	ReleasedUniquePtr<ID3D12Resource>	m_Resource;
	ResourceState						m_CurrentState;

private:
	class UploadResource
	{
	public:
		UploadResource(const D3D12_RESOURCE_DESC& desc, void* data, size_t dataSize);
		~UploadResource() = default;

		bool IsResident() const { m_Fence.Ready(); }
		void WaitForUpload() { m_Fence.Wait(); }

		ID3D12Resource* Resource() { return m_StagedResource.get(); }

	private:
		ReleasedUniquePtr<ID3D12Resource>	m_StagedResource;
		DXFence								m_Fence;
	};

	std::unique_ptr<UploadResource>		m_StagingResource;
};
