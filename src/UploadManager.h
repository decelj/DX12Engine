#pragma once

#include "stdafx.h"
#include "DXCommandList.h"
#include "DXFence.h"

#include <d3d12.h>
#include <mutex>

class Resource;

class UploadManager
{
public:
	UploadManager();
	~UploadManager();

	void UploadDataTo(void* data, size_t dataSize, Resource& dest);
	void MakeAllResident();
	void WaitForUpload();

private:
	class UploadResource
	{
	public:
		UploadResource(const D3D12_RESOURCE_DESC& desc, void* data, size_t dataSize);
		~UploadResource() = default;

		ID3D12Resource* Resource() { return m_StagedResource.get(); }

	private:
		ReleasedUniquePtr<ID3D12Resource>	m_StagedResource;
	};
	using UploadResourceUniquePtr = std::unique_ptr<UploadResource>;

	DXCommandList							m_UploadCommands;
	DXFence									m_Fence;
	std::vector<UploadResourceUniquePtr>	m_PendingResources;
	std::vector<UploadResourceUniquePtr>	m_SubmittedResources;

	std::mutex								m_SubmitLock;
};
