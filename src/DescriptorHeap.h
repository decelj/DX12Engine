#pragma once

#include "stdafx.h"
#include <d3d12.h>

class DXDevice;

enum class DescriptorType
{
	RTV,
	SRV
};

struct DescriptorHandle
{
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
};

struct DescriptorHandleWithIdx : public DescriptorHandle
{
	uint32_t idx;
};

class DescriptorHeap
{
public:
	DescriptorHeap(DXDevice& device, DescriptorType type, uint32_t maxCount);
	~DescriptorHeap();

	DescriptorHandleWithIdx AllocateHandle();
	void FreeHandle(const DescriptorHandleWithIdx& desc);

private:
	std::vector<uint32_t>					m_FreeHandles;
	ReleasedUniquePtr<ID3D12DescriptorHeap>	m_Heap;
	uint32_t								m_HandleSize;
};
