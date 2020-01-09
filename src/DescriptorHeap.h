#pragma once

#include "stdafx.h"
#include <d3d12.h>

class DXDevice;

enum class DescriptorType
{
	RTV = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	SRV = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	DSV = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
	Sampler = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
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
	~DescriptorHeap() = default;

	DescriptorHandleWithIdx AllocateHandle();
	void FreeHandle(const DescriptorHandleWithIdx& desc);

	ID3D12DescriptorHeap& Native() { return *m_Heap; }

private:
	std::vector<uint32_t>					m_FreeHandles;
	ReleasedUniquePtr<ID3D12DescriptorHeap>	m_Heap;
	uint32_t								m_HandleSize;
};
