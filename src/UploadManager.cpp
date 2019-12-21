#include "stdafx.h"
#include "UploadManager.h"
#include "DXDevice.h"

UploadManager::UploadManager()
	: m_UploadCommands(CommandType::COPY)
{
}

UploadManager::~UploadManager()
{
}

void UploadManager::UploadDataTo(const void* data, size_t dataSize, Resource& dest)
{
	if (!m_UploadCommands.IsOpen())
	{
		m_UploadCommands.Begin();
	}

	m_PendingResources.emplace_back(std::make_unique<UploadResource>(dest.GetDesc(), data, dataSize));
	m_UploadCommands.Native()->CopyResource(dest.Native(), m_PendingResources.back()->Resource());
	dest.SetStateTo(ResourceState::Common);
}

void UploadManager::MakeAllResident()
{
	WaitForUpload();

	std::lock_guard<std::mutex> lock(m_SubmitLock);

	m_UploadCommands.End();
	m_UploadCommands.Submit();
	m_Fence.Signal(CommandType::COPY);

	assert(m_SubmittedResources.empty());
	m_SubmittedResources.swap(m_PendingResources);
}

void UploadManager::WaitForUpload()
{
	m_Fence.Wait();
	m_SubmittedResources.clear();
}

UploadManager::UploadResource::UploadResource(const D3D12_RESOURCE_DESC& desc, const void* data, size_t dataSize)
	: m_StagedResource(nullptr)
{
	assert(data);
	assert(dataSize);

	m_StagedResource.reset(DXDevice::Instance().CreateCommitedUploadResource(desc, data, dataSize));
}
