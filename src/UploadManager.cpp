#include "stdafx.h"
#include "UploadManager.h"
#include "DXDevice.h"
#include "Texture.h"
#include "Resource.h"


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

	UploadBufferUniquePtr uploadBuffer = std::make_unique<UploadBuffer>(dataSize);

	uint8_t* gpuData = uploadBuffer->Map();
	std::memcpy(gpuData, data, dataSize);
	uploadBuffer->Unmap();

	m_UploadCommands.Native()->CopyResource(dest.Native(), uploadBuffer->Native());
	dest.SetStateTo(ResourceState::Common);

	m_PendingResources.emplace_back(std::move(uploadBuffer));
}

void UploadManager::UploadDataTo(const void* data, uint32_t dataBPT, Texture2D& dest)
{
	if (!m_UploadCommands.IsOpen())
	{
		m_UploadCommands.Begin();
	}

	const size_t uploadBufferSize = GetRequiredIntermediateSize(dest.Native(), 0, 1u);
	UploadBufferUniquePtr uploadBuffer = std::make_unique<UploadBuffer>(uploadBufferSize);

	const D3D12_RESOURCE_DESC destDesc = dest.GetDesc();
	D3D12_SUBRESOURCE_DATA subResource = {};
	subResource.pData = data;
	subResource.RowPitch = destDesc.Width * dataBPT;
	subResource.SlicePitch = subResource.RowPitch * destDesc.Height;
	UpdateSubresources(m_UploadCommands.Native(), dest.Native(), uploadBuffer->Native(), 0, 0, 1, &subResource);

	m_PendingResources.emplace_back(std::move(uploadBuffer));
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

UploadManager::UploadBuffer::UploadBuffer(size_t size)
	: m_Resource(nullptr)
{
	m_Resource.reset(DXDevice::Instance().CreateCommitedUploadResource(CD3DX12_RESOURCE_DESC::Buffer(size)));
}
