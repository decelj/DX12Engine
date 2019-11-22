#include "stdafx.h"
#include "DXDevice.h"
#include "Resource.h"
#include "DXCommandList.h"

Resource::Resource(ResourceDimension dimension, DXGI_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, void* data, size_t dataSize, ResourceState initialState)
	: m_Resource(nullptr)
	, m_CurrentState(initialState)
	, m_StagingResource(nullptr)
{
	DXDevice& device = DXDevice::Instance();
	m_Resource.reset(
		device.CreateCommitedResource(
			dimension, format, width, height, depth, static_cast<D3D12_RESOURCE_STATES>(initialState)
		));

	if (data)
	{
		assert(dataSize);
		assert(initialState == ResourceState::CopyDest);
		m_StagingResource = std::make_unique<UploadResource>(m_Resource->GetDesc(), data, dataSize);
	}
}

Resource::Resource(ResourceState initialState)
	: m_Resource(nullptr)
	, m_CurrentState(initialState)
	, m_StagingResource(nullptr)
{
}

void Resource::TransitionTo(ResourceState destState, DXCommandList& cmdList)
{
	if (destState == m_CurrentState)
	{
		return;
	}

	D3D12_RESOURCE_BARRIER barrier = CreateBarrier(m_Resource.get(), m_CurrentState, destState);
	m_CurrentState = destState;

	cmdList.Native()->ResourceBarrier(1, &barrier);
}

void Resource::MakeResident()
{
	if (m_StagingResource != nullptr)
	{
		m_StagingResource->WaitForUpload();
		m_StagingResource.reset();
	}
}

Resource::UploadResource::UploadResource(const D3D12_RESOURCE_DESC& desc, void* data, size_t dataSize)
	: m_StagedResource(nullptr)
{
	assert(data);
	assert(dataSize);

	m_StagedResource.reset(DXDevice::Instance().CreateCommitedUploadResource(desc, data, dataSize));
	m_Fence.Signal();
}
