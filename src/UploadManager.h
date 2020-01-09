#pragma once

#include "stdafx.h"
#include "DXCommandList.h"
#include "DXFence.h"
#include "d3dx12.h"

#include <d3d12.h>
#include <mutex>

class Resource;

class UploadManager
{
public:
	UploadManager();
	~UploadManager();

	void UploadDataTo(const void* data, size_t dataSize, Resource& dest);
	void MakeAllResident();
	void WaitForUpload();

private:
	class UploadBuffer
	{
	public:
		UploadBuffer(size_t size);
		~UploadBuffer() = default;

		ID3D12Resource* Native() { return m_Resource.get(); }

		uint8_t* Map()
		{
			uint8_t* gpuData = nullptr;
			m_Resource->Map(0, &CD3DX12_RANGE(0, 0), reinterpret_cast<void**>(&gpuData));
			return gpuData;
		}

		void Unmap()
		{
			m_Resource->Unmap(0, nullptr);
		}

	private:
		ReleasedUniquePtr<ID3D12Resource>	m_Resource;
	};
	using UploadBufferUniquePtr = std::unique_ptr<UploadBuffer>;

	DXCommandList							m_UploadCommands;
	DXFence									m_Fence;
	std::vector<UploadBufferUniquePtr>		m_PendingResources;
	std::vector<UploadBufferUniquePtr>		m_SubmittedResources;

	std::mutex								m_SubmitLock;
};
